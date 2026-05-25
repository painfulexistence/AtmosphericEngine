// file_system.cpp — Platform-agnostic async file I/O with transparent caching.
//
// Web (Emscripten)
//   ReadAsync  : emscripten_fetch → IndexedDB → WASM heap → callback
//   Prefetch   : batch fetch; text files also written to MEMFS so
//                fopen() / Lua require() / std::filesystem::exists() work.
//
// Native (Linux / macOS / Windows)
//   ReadAsync  : synchronous fread; callback fires before ReadAsync returns.
//   Prefetch   : parallel fread via JobSystem; onDone fires before Prefetch
//                returns (synchronous completion on native).
//
// The in-process cache (g_cache) is a file-scope global so it is reachable
// from C-style callbacks (Emscripten) and JobSystem lambdas without needing
// a captured 'this' pointer.

#include "file_system.hpp"
#include "console.hpp"

#include <filesystem>
#include <mutex>
#include <unordered_map>
#include <cstdio>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// In-process cache
// ─────────────────────────────────────────────────────────────────────────────
static std::unordered_map<std::string, FileSystem::Bytes> g_cache;
// Protects g_cache on native builds where JobSystem worker threads write it.
// On Emscripten (single-threaded), the mutex is a no-op but keeps the code
// correct if pthreads are ever enabled.
static std::mutex g_cacheMutex;

// ─────────────────────────────────────────────────────────────────────────────
// Singleton
// ─────────────────────────────────────────────────────────────────────────────
FileSystem* FileSystem::_instance = nullptr;

FileSystem& FileSystem::Get() {
    if (!_instance) _instance = new FileSystem();
    return *_instance;
}

// ─────────────────────────────────────────────────────────────────────────────
// Shared helper: synchronous read from the OS filesystem (or Emscripten MEMFS).
// Returns an empty vector on error.
// ─────────────────────────────────────────────────────────────────────────────
static FileSystem::Bytes ReadFromDisk(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return {};
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return {}; }
    long len = ftell(f);
    rewind(f);
    if (len <= 0) { fclose(f); return {}; }
    FileSystem::Bytes buf(static_cast<size_t>(len));
    if (fread(buf.data(), 1, static_cast<size_t>(len), f) != static_cast<size_t>(len)) {
        fclose(f); return {};
    }
    fclose(f);
    return buf;
}

// ─────────────────────────────────────────────────────────────────────────────
// Cache helpers — identical on all platforms
// ─────────────────────────────────────────────────────────────────────────────
bool FileSystem::IsCached(const std::string& path) const {
    std::lock_guard<std::mutex> lk(g_cacheMutex);
    return g_cache.count(path) != 0;
}

void FileSystem::EvictCache(const std::string& path) {
    std::lock_guard<std::mutex> lk(g_cacheMutex);
    g_cache.erase(path);
}

void FileSystem::ClearCache() {
    std::lock_guard<std::mutex> lk(g_cacheMutex);
    g_cache.clear();
}

bool FileSystem::Exists(const std::string& path) const {
    if (IsCached(path)) return true;
    return std::filesystem::exists(path);
}

FileSystem::Bytes FileSystem::ReadSync(const std::string& path) {
    {
        std::lock_guard<std::mutex> lk(g_cacheMutex);
        auto it = g_cache.find(path);
        if (it != g_cache.end()) return it->second; // copy from cache
    }
#ifdef __EMSCRIPTEN__
    // On web a cache-miss means Prefetch was not called — log and fail.
    ENGINE_LOG("[FileSystem] ReadSync cache miss on web: '{}' — call Prefetch first", path);
    return {};
#else
    auto bytes = ReadFromDisk(path);
    if (bytes.empty())
        ENGINE_LOG("[FileSystem] ReadSync: failed to read '{}'", path);
    return bytes;
#endif
}

FileSystem::Bytes FileSystem::ConsumeSync(const std::string& path) {
    {
        std::lock_guard<std::mutex> lk(g_cacheMutex);
        auto it = g_cache.find(path);
        if (it != g_cache.end()) {
            FileSystem::Bytes result = std::move(it->second);
            g_cache.erase(it);
            return result;
        }
    }
#ifndef __EMSCRIPTEN__
    // Native: fall back to disk read on cache miss.
    return ReadFromDisk(path);
#else
    ENGINE_LOG("[FileSystem] ConsumeSync cache miss on web: '{}' — call Prefetch first", path);
    return {};
#endif
}

// ═════════════════════════════════════════════════════════════════════════════
// Platform-specific implementations
// ═════════════════════════════════════════════════════════════════════════════

#ifdef __EMSCRIPTEN__
// ─────────────────────────────────────────────────────────────────────────────
// Web / Emscripten implementation
// ─────────────────────────────────────────────────────────────────────────────
#include <emscripten/fetch.h>
#include <emscripten.h>
#include <atomic>
#include <memory>

// ── MEMFS write helper ────────────────────────────────────────────────────────
// Runs inside the WASM module's JS closure; HEAPU8, FS, UTF8ToString are
// available without a Module. prefix.
EM_JS(void, fs_js_write_memfs, (const char* path_ptr, const uint8_t* data_ptr, int data_len), {
    var path  = UTF8ToString(path_ptr);
    // Recursively create parent directories (ignores "already exists")
    var parts = path.split('/');
    var dir   = '';
    for (var i = 0; i < parts.length - 1; ++i) {
        dir += (i === 0 && parts[i] === '' ? '' : '/') + parts[i];
        if (dir !== '') { try { FS.mkdir(dir); } catch(e) {} }
    }
    // Write (creates or overwrites); HEAPU8.subarray is a JS view, no extra copy
    FS.writeFile(path, HEAPU8.subarray(data_ptr, data_ptr + data_len));
});

// ── Text-file detection ───────────────────────────────────────────────────────
// Files with these extensions are written to MEMFS after fetching so that
// fopen() / Lua require() / std::filesystem::exists() find them.
static bool NeedsMemFS(const std::string& path) {
    static const char* kTextExts[] = {
        ".lua", ".json", ".glsl", ".vert", ".frag", ".tesc", ".tese",
        ".txt",  ".csv", ".xml",  ".yaml", ".toml",
    };
    for (const char* ext : kTextExts) {
        size_t elen = strlen(ext);
        if (path.size() >= elen &&
            path.compare(path.size() - elen, elen, ext) == 0)
            return true;
    }
    return false;
}

// ── Async batch state ─────────────────────────────────────────────────────────
namespace {

struct BatchState {
    std::atomic<int>               remaining{0};
    FileSystem::CompletionCallback onComplete;
};

struct FetchCtx {
    std::string                  path;
    bool                         writeToMemFS;
    FileSystem::ReadCallback     singleCB; // set for ReadAsync, null for Prefetch
    std::shared_ptr<BatchState>  batch;    // set for Prefetch, null for ReadAsync
};

void EM_OnSuccess(emscripten_fetch_t* f) {
    auto* ctx = static_cast<FetchCtx*>(f->userData);

    // Build byte vector from the fetch buffer
    FileSystem::Bytes bytes;
    if (f->numBytes > 0) {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(f->data);
        bytes.assign(data, data + static_cast<size_t>(f->numBytes));
    }

    if (!bytes.empty()) {
        // Optionally mirror to MEMFS for text-format assets
        if (ctx->writeToMemFS) {
            fs_js_write_memfs(ctx->path.c_str(), bytes.data(), (int)bytes.size());
            ENGINE_LOG("[FileSystem] MEMFS + cache: '{}' ({} bytes)", ctx->path, f->numBytes);
        } else {
            ENGINE_LOG("[FileSystem] Cached: '{}' ({} bytes)", ctx->path, f->numBytes);
        }
        // Store in in-process cache
        {
            std::lock_guard<std::mutex> lk(g_cacheMutex);
            g_cache[ctx->path] = bytes; // intentional copy; bytes also given to singleCB below
        }
    }

    // Fire single-read callback (ReadAsync path) — outside the lock
    if (ctx->singleCB) ctx->singleCB(std::move(bytes), true);

    // Batch completion (Prefetch path)
    if (ctx->batch && --ctx->batch->remaining == 0)
        if (ctx->batch->onComplete) ctx->batch->onComplete();

    emscripten_fetch_close(f);
    delete ctx;
}

void EM_OnError(emscripten_fetch_t* f) {
    auto* ctx = static_cast<FetchCtx*>(f->userData);
    ENGINE_LOG("[FileSystem] HTTP {} — failed to fetch '{}'", f->status, f->url);
    if (ctx->singleCB) ctx->singleCB({}, false);
    // Batch errors are non-fatal; decrement so Prefetch can still complete.
    if (ctx->batch && --ctx->batch->remaining == 0)
        if (ctx->batch->onComplete) ctx->batch->onComplete();
    emscripten_fetch_close(f);
    delete ctx;
}

void LaunchFetch(const std::string& path, FetchCtx* ctx) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    // LOAD_TO_MEMORY : make fetch->data available in the callback
    // PERSIST_FILE   : cache bytes in IndexedDB; subsequent loads skip network
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;
    attr.onsuccess  = EM_OnSuccess;
    attr.onerror    = EM_OnError;
    attr.userData   = ctx;
    emscripten_fetch(&attr, path.c_str());
}

} // anonymous namespace

// ── Public API (web) ──────────────────────────────────────────────────────────

void FileSystem::ReadAsync(const std::string& path, ReadCallback cb) {
    // Cache hit → immediate
    {
        std::lock_guard<std::mutex> lk(g_cacheMutex);
        auto it = g_cache.find(path);
        if (it != g_cache.end()) { cb(it->second, true); return; }
    }
    // Cache miss → fetch asynchronously; callback fires in the browser event loop
    auto* ctx = new FetchCtx{path, NeedsMemFS(path), std::move(cb), nullptr};
    LaunchFetch(path, ctx);
}

void FileSystem::Prefetch(const std::vector<std::string>& paths,
                          CompletionCallback onDone) {
    if (paths.empty()) { if (onDone) onDone(); return; }

    // Skip paths that are already in cache
    std::vector<std::string> pending;
    {
        std::lock_guard<std::mutex> lk(g_cacheMutex);
        for (const auto& p : paths)
            if (!g_cache.count(p)) pending.push_back(p);
    }
    if (pending.empty()) { if (onDone) onDone(); return; }

    auto batch        = std::make_shared<BatchState>();
    batch->remaining  = static_cast<int>(pending.size());
    batch->onComplete = std::move(onDone);

    for (const auto& p : pending) {
        auto* ctx = new FetchCtx{p, NeedsMemFS(p), nullptr, batch};
        LaunchFetch(p, ctx);
    }
    // Returns immediately; onDone fires asynchronously via the browser event loop
}

#else
// ─────────────────────────────────────────────────────────────────────────────
// Native implementation (Linux / macOS / Windows)
// ─────────────────────────────────────────────────────────────────────────────
#include "job_system.hpp"

void FileSystem::ReadAsync(const std::string& path, ReadCallback cb) {
    // Cache hit → immediate
    {
        std::lock_guard<std::mutex> lk(g_cacheMutex);
        auto it = g_cache.find(path);
        if (it != g_cache.end()) { cb(it->second, true); return; }
    }
    // Native: synchronous disk read; callback fires before ReadAsync returns
    auto bytes = ReadFromDisk(path);
    bool ok    = !bytes.empty();
    if (ok) {
        std::lock_guard<std::mutex> lk(g_cacheMutex);
        g_cache[path] = bytes;
    } else {
        ENGINE_LOG("[FileSystem] ReadAsync: failed to read '{}'", path);
    }
    cb(std::move(bytes), ok);
}

void FileSystem::Prefetch(const std::vector<std::string>& paths,
                          CompletionCallback onDone) {
    if (paths.empty()) { if (onDone) onDone(); return; }

    // Parallel disk reads via JobSystem worker threads
    for (const auto& p : paths) {
        if (IsCached(p)) continue; // already cached, skip
        auto pathCopy = p;
        JobSystem::Get()->Execute([pathCopy](int /*threadID*/) {
            auto bytes = ReadFromDisk(pathCopy);
            if (!bytes.empty()) {
                std::lock_guard<std::mutex> lk(g_cacheMutex);
                g_cache[pathCopy] = std::move(bytes);
            } else {
                ENGINE_LOG("[FileSystem] Prefetch: failed to read '{}'", pathCopy);
            }
        });
    }
    JobSystem::Get()->Wait(); // blocks until all reads complete

    // onDone fires synchronously on native (before Prefetch returns)
    if (onDone) onDone();
}

#endif // __EMSCRIPTEN__

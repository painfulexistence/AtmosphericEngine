#ifdef __EMSCRIPTEN__
#include "web_asset_fetcher.hpp"
#include "asset_manager.hpp"
#include "console.hpp"

#include <emscripten/fetch.h>
#include <emscripten.h>        // EM_JS
#include <atomic>
#include <memory>

// ─────────────────────────────────────────────────────────────────────────────
// JS helper: write raw bytes into Emscripten's virtual filesystem (MEMFS).
//
// Called from PreloadToMEMFS after a successful fetch so that standard POSIX
// fopen() / Lua's io / std::filesystem can find the file.
//
// The JS runs inside the WASM module's closure so HEAPU8, FS, and UTF8ToString
// are available without a Module. prefix.
// ─────────────────────────────────────────────────────────────────────────────
EM_JS(void, js_memfs_write_file, (const char* path_ptr, const uint8_t* data_ptr, int data_len), {
    var path = UTF8ToString(path_ptr);
    // Recursively create parent directories (silently ignores "already exists")
    var parts = path.split('/');
    var dir   = '';
    for (var i = 0; i < parts.length - 1; ++i) {
        dir += (i === 0 && parts[i] === '' ? '' : '/') + parts[i];
        if (dir !== '') {
            try { FS.mkdir(dir); } catch(e) {}
        }
    }
    // Write the file (creates or overwrites)
    var data = HEAPU8.subarray(data_ptr, data_ptr + data_len);
    FS.writeFile(path, data);
});

// ─────────────────────────────────────────────────────────────────────────────
// Internal helpers
// ─────────────────────────────────────────────────────────────────────────────
namespace {

// Shared state for a batch call; freed when the last fetch settles.
struct BatchState {
    std::atomic<int>      remaining{0};
    std::function<void()> onComplete;
};

// Per-fetch heap allocation; freed in the callbacks.
struct FetchCtx {
    std::function<void(const uint8_t*, size_t)> onSuccess;
    std::function<void(const std::string&)>     onError;
    std::shared_ptr<BatchState>                  batch; // null for one-shot
};

void OnSuccess(emscripten_fetch_t* f) {
    auto* ctx = static_cast<FetchCtx*>(f->userData);

    if (ctx->onSuccess)
        ctx->onSuccess(reinterpret_cast<const uint8_t*>(f->data),
                       static_cast<size_t>(f->numBytes));

    if (ctx->batch && --ctx->batch->remaining == 0)
        if (ctx->batch->onComplete)
            ctx->batch->onComplete();

    emscripten_fetch_close(f);
    delete ctx;
}

void OnError(emscripten_fetch_t* f) {
    auto* ctx = static_cast<FetchCtx*>(f->userData);

    ENGINE_LOG("[WebAssetFetcher] HTTP {} -- failed to fetch '{}'", f->status, f->url);

    if (ctx->onError)
        ctx->onError(std::string(f->url));

    // Batch errors are non-fatal: decrement so the game can still start.
    if (ctx->batch && --ctx->batch->remaining == 0)
        if (ctx->batch->onComplete)
            ctx->batch->onComplete();

    emscripten_fetch_close(f);
    delete ctx;
}

void LaunchFetch(const std::string& url, FetchCtx* ctx) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    // LOAD_TO_MEMORY : make fetch->data available in the callback
    // PERSIST_FILE   : cache bytes in IndexedDB (keyed by URL)
    //                  next load reads from IDB, skipping the network
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;
    attr.onsuccess  = OnSuccess;
    attr.onerror    = OnError;
    attr.userData   = ctx;
    emscripten_fetch(&attr, url.c_str());
}

} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void WebAssetFetcher::Fetch(
    const std::string& url,
    std::function<void(const uint8_t*, size_t)> onSuccess,
    std::function<void(const std::string&)>     onError)
{
    auto* ctx = new FetchCtx{std::move(onSuccess), std::move(onError), nullptr};
    LaunchFetch(url, ctx);
}

// ─────────────────────────────────────────────────────────────────────────────

void WebAssetFetcher::Preload(
    const std::vector<std::string>& urls,
    std::function<void()> onAllComplete)
{
    if (urls.empty()) { if (onAllComplete) onAllComplete(); return; }

    auto batch        = std::make_shared<BatchState>();
    batch->remaining  = static_cast<int>(urls.size());
    batch->onComplete = std::move(onAllComplete);

    for (const auto& url : urls) {
        auto* ctx = new FetchCtx{
            // onSuccess: hand bytes to AssetManager (e.g. for KTX2 transcoding)
            [url](const uint8_t* data, size_t size) {
                AssetManager::Get().StorePreloadedAsset(
                    url, std::vector<uint8_t>(data, data + size));
                ENGINE_LOG("[WebAssetFetcher] Preload cached '{}' ({} KiB)",
                           url, size / 1024);
            },
            /*onError=*/nullptr,
            batch
        };
        LaunchFetch(url, ctx);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void WebAssetFetcher::PreloadToMEMFS(
    const std::vector<std::string>& urls,
    std::function<void()> onAllComplete)
{
    if (urls.empty()) { if (onAllComplete) onAllComplete(); return; }

    auto batch        = std::make_shared<BatchState>();
    batch->remaining  = static_cast<int>(urls.size());
    batch->onComplete = std::move(onAllComplete);

    for (const auto& url : urls) {
        auto* ctx = new FetchCtx{
            // onSuccess: write bytes to MEMFS so fopen() / Lua can read them.
            //
            // Memory note: the fetch buffer is live for the duration of this
            // lambda.  js_memfs_write_file uses HEAPU8.subarray (a JS view,
            // no extra copy), so peak allocation is just the file bytes once.
            // emscripten_fetch_close() frees the buffer after the callback.
            [url](const uint8_t* data, size_t size) {
                js_memfs_write_file(url.c_str(), data, static_cast<int>(size));
                ENGINE_LOG("[WebAssetFetcher] MEMFS wrote '{}' ({} bytes)",
                           url, size);
            },
            /*onError=*/nullptr,
            batch
        };
        LaunchFetch(url, ctx);
    }
}

#endif // __EMSCRIPTEN__

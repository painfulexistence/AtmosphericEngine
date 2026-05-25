#pragma once
#ifdef __EMSCRIPTEN__

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// WebAssetFetcher
//
// Async asset loader for WebAssembly / WebGL2 builds.
// Uses emscripten_fetch with EMSCRIPTEN_FETCH_PERSIST_FILE so every successful
// download is written to the browser's IndexedDB. Subsequent page-loads (or
// offline use) skip the network and read from the local cache instead.
//
// Two loading strategies
// ──────────────────────
//
//  Preload()          -> stores bytes in AssetManager::_webAssetCache
//                       consumed by LoadKTX2Texture() -> glCompressedTexImage2D
//                       Best for: binary GPU assets (KTX2 textures, audio, ...)
//
//  PreloadToMEMFS()   -> writes bytes into Emscripten's in-process virtual FS
//                       (MEMFS), making them accessible to standard fopen() /
//                       std::filesystem::exists() calls made by C or Lua.
//                       Best for: text assets that are read by third-party code
//                       we cannot instrument (Lua scripts, JSON configs, ...).
//
// Typical usage in main() for a Lua web build:
//
//   WebAssetFetcher::Preload(
//     { "assets/textures/foo.ktx2" },          // -> AssetManager cache
//     []() { ... });
//
//   WebAssetFetcher::PreloadToMEMFS(
//     { "assets/scripts/main.lua",              // -> MEMFS (fopen-visible)
//       "assets/scripts/components/player.lua" },
//     []() { StartGame(); });
// ─────────────────────────────────────────────────────────────────────────────
class WebAssetFetcher {
public:
    // ── Single-file helper ────────────────────────────────────────────────────

    // Fetch one URL and deliver its raw bytes to onSuccess.
    // On failure the HTTP status is logged; onError (optional) receives the URL.
    static void Fetch(
        const std::string& url,
        std::function<void(const uint8_t*, size_t)> onSuccess,
        std::function<void(const std::string&)>     onError = nullptr);

    // ── Batch helpers (preferred for startup preloading) ──────────────────────

    // Fetch a list of URLs and store each in AssetManager::_webAssetCache.
    // onAllComplete fires once every request has settled (success or error).
    // Assets already in IndexedDB are returned without a network round-trip.
    static void Preload(
        const std::vector<std::string>& urls,
        std::function<void()> onAllComplete);

    // Fetch a list of URLs and write each response directly into Emscripten's
    // MEMFS (the in-process virtual filesystem).
    //
    // After this call completes, the files are readable by:
    //   * std::filesystem::exists() / std::ifstream
    //   * POSIX fopen() / fread()
    //   * Lua's io library, require(), safe_script_file()
    //
    // Parent directories are created automatically (equivalent to mkdir -p).
    // Files are also cached in IndexedDB so subsequent page-loads are instant.
    //
    // NOTE: MEMFS is in-process only -- it does not persist across page reloads.
    //       The IndexedDB layer provides persistence; MEMFS is just the bridge
    //       that makes files visible to C/Lua file APIs within the current run.
    static void PreloadToMEMFS(
        const std::vector<std::string>& urls,
        std::function<void()> onAllComplete);
};

#endif // __EMSCRIPTEN__

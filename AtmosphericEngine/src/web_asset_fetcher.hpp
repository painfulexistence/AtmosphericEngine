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
//
// Design goals
// ────────────
//   • Eliminate the `--preload-file` `.data` bundle for large binary assets
//     (primarily textures).  The `.data` bundle forces ALL asset bytes into
//     WASM linear memory at startup — even files not yet needed.
//
//   • Use emscripten_fetch with EMSCRIPTEN_FETCH_PERSIST_FILE so that every
//     successfully downloaded file is written to the browser's IndexedDB.
//     Subsequent page-loads read from IndexedDB instead of the network, which
//     gives near-instant loads after the first visit.
//
// Memory lifecycle (per KTX2 texture)
// ────────────────────────────────────
//   1. emscripten_fetch downloads KTX2 bytes  → in WASM heap (fetch buffer)
//   2. OnSuccess copies them to AssetManager::_webAssetCache
//   3. emscripten_fetch_close() frees the fetch buffer immediately
//   4. AssetManager::LoadKTX2Texture() transcodes from _webAssetCache
//      → ETC2 block buffer (4× smaller than RGBA)
//   5. glCompressedTexImage2D uploads to GPU
//   6. Both the _webAssetCache entry and the ETC2 buffer are freed
//
//   Peak per-texture heap usage ≈ ktx2_file_size + etc2_blocks_size
//   vs. the old approach ≈ ALL textures in .data + rgba_decoded_size
//
// Typical usage (in main() before entering the game loop)
// ────────────────────────────────────────────────────────
//   WebAssetFetcher::Preload(
//     { "assets/textures/foo.ktx2", "assets/textures/bar.ktx2" },
//     []() {
//         // All bytes are now in AssetManager's web cache.
//         // Start the actual game loop here.
//         static MyGame game({...});
//         game.Run();
//     });
//   // main() returns immediately; browser event loop takes over.
// ─────────────────────────────────────────────────────────────────────────────
class WebAssetFetcher {
public:
    // Fetch one URL and deliver its raw bytes to onSuccess.
    // On failure the HTTP status is logged; onError (optional) receives the URL.
    static void Fetch(
        const std::string& url,
        std::function<void(const uint8_t*, size_t)> onSuccess,
        std::function<void(const std::string&)>     onError = nullptr);

    // Fetch a list of URLs and store each in AssetManager's web cache via
    // AssetManager::StorePreloadedAsset().
    // onAllComplete fires once every request has settled (success or error).
    // Assets already cached in IndexedDB are returned without a network hop.
    static void Preload(
        const std::vector<std::string>& urls,
        std::function<void()> onAllComplete);
};

#endif // __EMSCRIPTEN__

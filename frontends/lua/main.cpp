/// AtmosLua - Love2D-style Lua Game Runtime
///
/// This is the entry point for running Lua games with the Atmospheric Engine.
/// It creates a LuaApplication that loads and executes main.lua.
///
/// Usage:
///   ./AtmosLua [options]
///
/// The runtime looks for main.lua in the following locations (in order):
///   1. ./assets/scripts/main.lua
///   2. ./assets/main.lua
///   3. ./main.lua
///
/// Example main.lua:
///   function load()
///       player = atmos.world.spawn(0, 1, 0)
///       player.name = "Player"
///   end
///
///   function update(dt)
///       if atmos.input.isKeyDown(atmos.keys.ESCAPE) then
///           atmos.quit()
///       end
///   end

#include "lua_application.hpp"

#ifdef __EMSCRIPTEN__
// WebAssetFetcher: emscripten_fetch-based loader with IndexedDB persistence.
// Two strategies are used at startup:
//
//   Preload()        -> AssetManager::_webAssetCache
//                       consumed by LoadKTX2Texture (GPU-compressed textures)
//
//   PreloadToMEMFS() -> Emscripten MEMFS (in-process virtual filesystem)
//                       makes files visible to fopen() and Lua's io / require()
//
// The two-stage init below runs them in sequence so that by the time
// LuaApplication::OnInit() is called, both the texture cache and MEMFS are
// fully populated.
#include "web_asset_fetcher.hpp"
#include <vector>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// Asset manifests
//
// IMPORTANT: every .lua file that can be require()'d at runtime must appear
// in kScriptURLs.  Lua's require() searches package.path (fopen) at runtime,
// so any missing file will cause a "module not found" error mid-game.
//
// Texture files must be KTX2 format; convert with:
//   basisu -ktx2 -mipmap source.jpg
//   toktx --t2 --encode etc1s --mipmap out.ktx2 source.jpg
// ─────────────────────────────────────────────────────────────────────────────

// KTX2 textures -> loaded into AssetManager::_webAssetCache (Preload)
static const std::vector<std::string> kTextureURLs = {
    "assets/textures/default_diff.ktx2",
    "assets/textures/default_norm.ktx2",
    "assets/textures/default_ao.ktx2",
    "assets/textures/default_rough.ktx2",
    "assets/textures/default_metallic.ktx2",
};

// Lua scripts -> written to MEMFS (PreloadToMEMFS) so fopen/require work
// Add every .lua file your game uses (including those loaded via require()).
static const std::vector<std::string> kScriptURLs = {
    "assets/scripts/main.lua",
    // Add further modules here, e.g.:
    // "assets/scripts/components/player.lua",
    // "assets/scripts/utils/helpers.lua",
};

// ─────────────────────────────────────────────────────────────────────────────
// Two-stage async preload:
//
//   Stage 1  Preload(kTextureURLs)      -> _webAssetCache (KTX2 bytes)
//   Stage 2  PreloadToMEMFS(kScriptURLs) -> MEMFS files  (Lua-visible)
//   Stage 3  StartGame()
//
// Memory note
// -----------
// During Stage 1 each fetch buffer holds KTX2 bytes in WASM heap; after
// StorePreloadedAsset() the buffer is freed by emscripten_fetch_close().
//
// During Stage 2 each script is written to MEMFS via HEAPU8.subarray (JS
// view, no extra copy).  Script bytes are immediately freed after the FS
// write; MEMFS keeps its own copy (text files are tiny).
// ─────────────────────────────────────────────────────────────────────────────
static void StartGame();  // forward declaration

static void OnScriptsFetched() {
    // Stage 3: everything is ready -- start the engine.
    StartGame();
}

static void OnTexturesFetched() {
    // Stage 2: fetch Lua scripts into MEMFS.
    WebAssetFetcher::PreloadToMEMFS(kScriptURLs, OnScriptsFetched);
}

int main(int argc, char* argv[]) {
    // Stage 1: fetch KTX2 textures into AssetManager cache.
    // main() returns immediately; Emscripten's event loop keeps the page alive.
    WebAssetFetcher::Preload(kTextureURLs, OnTexturesFetched);
    return 0;
}

static void StartGame() {
    // At this point:
    //   _webAssetCache has all KTX2 bytes  -> LoadDefaultTextures() uses them
    //   MEMFS has all .lua files            -> LoadUserScripts() can fopen them
    static LuaApplication app({
        .windowTitle       = "AtmosLua",
        .windowWidth       = 1280,
        .windowHeight      = 720,
        .windowResizable   = true,
        .vsync             = true,
        .useDefaultTextures = true,
        .useDefaultShaders  = true,
    });
    app.Run(); // installs emscripten_set_main_loop; never returns
}

#else
// ─────────────────────────────────────────────────────────────────────────────
// Native entry point (Linux / macOS / Windows)
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    LuaApplication app({
        .windowTitle       = "AtmosLua",
        .windowWidth       = 1280,
        .windowHeight      = 720,
        .windowResizable   = true,
        .vsync             = true,
        .useDefaultTextures = true,
        .useDefaultShaders  = true,
    });
    app.Run();
    return 0;
}
#endif

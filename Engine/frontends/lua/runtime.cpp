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

#include "Atmospheric/file_system.hpp"
#include "lua_application.hpp"
#include "lua_diagnostics.hpp"

#include <string>
#include <vector>

static AppConfig RuntimeConfig() {
    return AppConfig{
        .windowTitle = "AtmosLua",
        .windowWidth = 1280,
        .windowHeight = 720,
        .windowResizable = true,
        .vsync = true,
        .useDefaultTextures = true,
        .useDefaultShaders = true,
    };
}

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

// ─────────────────────────────────────────────────────────────────────────────
// Web / WebAssembly entry point
//
// FileSystem::Prefetch handles everything in a single call:
//
//   KTX2 textures   → in-process cache, consumed by LoadKTX2Texture()
//   Lua scripts     → in-process cache + Emscripten MEMFS
//                     (FileSystem auto-detects .lua extension)
//                     makes files visible to fopen() and Lua's io/require()
//
// All files are also persisted in IndexedDB so subsequent page-loads skip
// the network entirely.
//
// Asset manifests
// ───────────────
// IMPORTANT: every .lua file that can be require()'d at runtime must appear
// in kAssets.  Lua's require() searches package.path (fopen) at runtime, so
// any missing file will cause a "module not found" error mid-game.
//
// Texture files must be KTX2 format; convert with:
//   basisu -ktx2 -mipmap source.jpg
//   toktx --t2 --encode etc1s --mipmap out.ktx2 source.jpg
// ─────────────────────────────────────────────────────────────────────────────

static const std::vector<std::string> kAssets = {
    // KTX2 textures (binary → cache only, consumed by LoadKTX2Texture)
    "assets/textures/default_diff.ktx2",
    "assets/textures/default_norm.ktx2",
    "assets/textures/default_ao.ktx2",
    "assets/textures/default_rough.ktx2",
    "assets/textures/default_metallic.ktx2",

    // Lua scripts (text → cache + MEMFS, so fopen/require work)
    // Add every .lua file your game uses, including require()'d modules:
    "assets/scripts/main.lua",
    // "assets/scripts/components/player.lua",
    // "assets/scripts/utils/helpers.lua",
};

// ─────────────────────────────────────────────────────────────────────────────
// Playground bridge
//
// shell.html (the Lua playground page) drives the runtime through the exports
// below and listens on three optional Module hooks:
//
//   Module.onLuaError(context, message)  a script error (compile or runtime)
//   Module.onLuaSceneLoaded()            scene (re)loaded and load() has run
//   Module.onLuaQuit()                   the script called atmos.quit()
//
// A plain Emscripten shell simply never defines them and gets the default
// behaviour (errors on stderr, quit closes the window).
// ─────────────────────────────────────────────────────────────────────────────

static LuaApplication* g_app = nullptr;
static std::string g_pendingSource;// latest source handed over by the page
static bool g_reloadQueued = false;// coalesces rapid Run clicks into one reload
static constexpr const char* kPlaygroundChunk = "=main.lua";

extern "C" {

    // Replace the running script with `source` (UTF-8, `length` bytes). The
    // chunk is compiled right away so a syntax error comes back synchronously
    // (returns 0, error already reported through Module.onLuaError) without
    // disturbing the running game; a valid chunk is applied at the start of
    // the next frame, outside any entity tick. Returns 1 when queued.
    EMSCRIPTEN_KEEPALIVE
    int ae_lua_run(const char* source, int length) {
        if (!g_app || !source || length < 0) return 0;

        std::string src(source, static_cast<size_t>(length));
        if (!g_app->CheckSyntax(src, kPlaygroundChunk)) return 0;

        g_pendingSource = std::move(src);
        if (!g_reloadQueued) {
            g_reloadQueued = true;
            g_app->DeferSpawn([]() {
                g_reloadQueued = false;
                g_app->ReloadScript(g_pendingSource, kPlaygroundChunk);
            });
        }
        return 1;
    }

    // 1 once the application exists (assets prefetched, engine running).
    EMSCRIPTEN_KEEPALIVE
    int ae_lua_is_ready() {
        return g_app != nullptr ? 1 : 0;
    }
}

static void StartGame();

int main(int argc, char* argv[]) {
    // Prefetch all assets in one call.
    //   • KTX2 files are stored in the FileSystem cache.
    //   • .lua files are stored in the cache AND written to MEMFS.
    //   • All files are persisted to IndexedDB for offline / fast reload.
    // main() returns immediately; Emscripten's event loop keeps the page alive.
    // StartGame() fires asynchronously once all fetches settle.
    FileSystem::Get().Prefetch(kAssets, StartGame);
    return 0;
}

static void StartGame() {
    // At this point:
    //   FileSystem cache has all KTX2 bytes  → LoadDefaultTextures() uses them
    //   MEMFS has all .lua files              → LoadUserScripts() can fopen them
    static LuaApplication app(RuntimeConfig());
    g_app = &app;

    // Page hooks — see "Playground bridge" above. All run on the main thread
    // (the engine's game loop is emscripten_set_main_loop, not a pthread).
    SetLuaErrorSink([](const std::string& context, const std::string& message) {
        // clang-format off
        EM_ASM({ if (Module.onLuaError) Module.onLuaError(UTF8ToString($0), UTF8ToString($1)); },
               context.c_str(), message.c_str());
        // clang-format on
    });
    app.SetSceneLoadedHandler([]() {
        // clang-format off
        EM_ASM({ if (Module.onLuaSceneLoaded) Module.onLuaSceneLoaded(); });
        // clang-format on
    });
    app.SetQuitHandler([]() {
        // The playground restarts the scene instead of tearing down the loop.
        // clang-format off
        const int handled = EM_ASM_INT({ return Module.onLuaQuit ? (Module.onLuaQuit(), 1) : 0; });
        // clang-format on
        if (!handled) g_app->Quit();
    });

    app.Run();// installs emscripten_set_main_loop; never returns
}

#else
// ─────────────────────────────────────────────────────────────────────────────
// Native entry point (Linux / macOS / Windows)
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    LuaApplication app(RuntimeConfig());
    app.Run();
    return 0;
}
#endif

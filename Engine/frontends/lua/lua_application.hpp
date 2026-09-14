#pragma once

#include "Atmospheric.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <functional>
#include <string>

/// LuaApplication - A Love2D-style Lua game runtime
///
/// This class provides a complete Lua scripting environment where Lua code
/// controls the game logic through callbacks (load, update, draw).
///
/// Usage:
///   1. Build AtmosLua executable
///   2. Place main.lua in assets/scripts/
///   3. Run AtmosLua
///
/// Lua callbacks:
///   - load()      : Called once after engine initialization
///   - update(dt)  : Called every frame with delta time
///   - draw()      : Called every frame for rendering (optional)
///   - keypressed(key)   : Called on key press
///   - keyreleased(key)  : Called on key release
///
/// Live reloading (used by the web playground): ReloadScript() swaps the whole
/// game for a new chunk at runtime — see the method for the exact sequence.
///
class LuaApplication : public Application {
public:
    explicit LuaApplication(AppConfig config = {});
    ~LuaApplication() override;

    /// Access the Lua state (for advanced usage)
    sol::state& GetLuaState() {
        return _lua;
    }

    /// Compile `source` without running it. A syntax error is reported through
    /// ReportLuaError() and returns false; the running game is left untouched.
    /// `chunkName` follows Lua's convention ("=main.lua" shows up as
    /// "main.lua:12:" in error messages).
    bool CheckSyntax(const std::string& source, const std::string& chunkName);

    /// Replace the running game with `source`: destroys every GameObject the
    /// current script spawned, unloads the scene, rebuilds the Lua state from
    /// scratch, re-binds the engine APIs, runs the chunk, then reloads the scene
    /// and calls load() again. Nothing from the previous run survives.
    ///
    /// Must run on the game thread outside an entity tick — queue it through
    /// DeferSpawn() when the request comes from outside the frame (JS, input).
    /// Returns false when the chunk failed to compile or raised while running.
    bool ReloadScript(const std::string& source, const std::string& chunkName);

    /// What atmos.quit() does. Default: Application::Quit(). The web playground
    /// restarts the scene instead of tearing down the main loop.
    void SetQuitHandler(std::function<void()> handler) {
        _quitHandler = std::move(handler);
    }

    /// Invoked after every scene (re)load, once Lua load() has run.
    void SetSceneLoadedHandler(std::function<void()> handler) {
        _sceneLoadedHandler = std::move(handler);
    }

protected:
    void OnInit() override;
    void OnLoad() override;
    void OnUpdate(float dt, float time) override;

private:
    sol::state _lua;

    // Cached Lua callbacks for performance
    sol::protected_function _luaLoad;
    sol::protected_function _luaUpdate;
    sol::protected_function _luaDraw;
    sol::protected_function _luaKeypressed;
    sol::protected_function _luaKeyreleased;

    std::function<void()> _quitHandler;
    std::function<void()> _sceneLoadedHandler;

    // load() has run for the current chunk. On the web GoScene() completes
    // asynchronously, so a frame can tick between a reload and its load().
    bool _scriptLoaded = false;

    /// Initialize Lua environment and open standard libraries
    void InitializeLua();

    /// Bind all engine APIs to Lua (atmos.* namespace)
    void BindEngineAPIs();

    /// Load and execute user scripts
    void LoadUserScripts();

    /// Cache Lua callback functions for efficient calling
    void CacheCallbacks();

    /// Drop every cached callback reference (must precede replacing _lua)
    void ResetCallbacks();

    /// Run a chunk in the current state; reports and returns false on error
    bool RunChunk(const std::string& source, const std::string& chunkName);

    /// Destroy the top-level GameObjects scripts spawned via atmos.world.spawn()
    void DestroyScriptEntities();

    /// (Re)create the "Default" material scene reloads throw away
    void EnsureDefaultMaterial();

    /// atmos.quit(): defer to the host's handler when one is installed
    void RequestQuit();

    /// Handle Lua errors with proper error messages
    void HandleError(const sol::protected_function_result& result, const std::string& context);

    /// Call a Lua function safely with error handling
    template<typename... Args> void CallLua(sol::protected_function& func, const std::string& name, Args&&... args) {
        auto result = func(std::forward<Args>(args)...);
        if (!result.valid()) {
            HandleError(result, name + "()");
        }
    }
};

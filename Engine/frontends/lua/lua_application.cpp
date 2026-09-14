#include "lua_application.hpp"
#include "Atmospheric/gfx_factory.hpp"
#include "lua_diagnostics.hpp"

#include "Atmospheric/file_system.hpp"
#include <chrono>
#include <filesystem>

// Forward declarations for binding functions
void BindCoreTypes(sol::state& lua);
void BindInputAPI(sol::state& lua, InputSubsystem* input);
void BindWorldAPI(sol::state& lua, LuaApplication* app);
void BindGraphicsAPI(sol::state& lua, GraphicsSubsystem* graphics);
void BindPhysicsAPI(sol::state& lua, LuaApplication* app);
void BindAudioAPI(sol::state& lua, AudioSubsystem* audio);

// ── Diagnostics (lua_diagnostics.hpp) ──────────────────────────────────────────
namespace {
    LuaErrorSink s_errorSink;
    bool s_halted = false;

    // Watchdog: a count hook that trips once the current callback has run past
    // its deadline. Lua re-enters the hook every kWatchdogInstructions VM
    // instructions, so `while true do end` in update() surfaces as an ordinary
    // script error instead of freezing the process — on the web the game loop
    // shares the page's main thread, so a stuck script would hang the whole tab.
    // The deadline is re-armed before every entry into Lua (frame, load, reload).
    constexpr int kWatchdogInstructions = 10000;
    constexpr int kFrameBudgetMs = 3000;
    constexpr int kLoadBudgetMs = 15000;
    std::chrono::steady_clock::time_point s_deadline = std::chrono::steady_clock::time_point::max();

    void WatchdogHook(lua_State* L, lua_Debug* /*ar*/) {
        if (std::chrono::steady_clock::now() > s_deadline) {
            luaL_error(L, "script ran for too long in a single frame (infinite loop?)");
        }
    }

    void ArmWatchdog(int milliseconds) {
        s_deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
    }

    // "=main.lua" / "@./assets/scripts/main.lua" → "main.lua"
    std::string DisplayName(const std::string& chunkName) {
        std::string name = chunkName;
        if (!name.empty() && (name[0] == '=' || name[0] == '@')) name.erase(0, 1);
        return std::filesystem::path(name).filename().string();
    }
}// namespace

void ReportLuaError(const std::string& context, const std::string& message) {
    fmt::print(stderr, "[Lua Error] {}: {}\n", context, message);
    if (s_errorSink) s_errorSink(context, message);
}

void SetLuaErrorSink(LuaErrorSink sink) {
    s_errorSink = std::move(sink);
}

bool IsLuaHalted() {
    return s_halted;
}

void SetLuaHalted(bool halted) {
    s_halted = halted;
}

// ── LuaApplication ─────────────────────────────────────────────────────────────
LuaApplication::LuaApplication(AppConfig config) : Application(config) {
}

LuaApplication::~LuaApplication() {
    // Destroy all GameObjects/Components before the sol::state _lua is destroyed.
    // This prevents ScriptableComponent's sol references from calling luaL_unref with a dangling lua_State* pointer.
    ArmWatchdog(kLoadBudgetMs);// onDestroy() callbacks run from the destructors
    DestroyEntities();
}

void LuaApplication::OnInit() {
    // Load default shaders and textures (skip in WebGPU mode — no GL context)
#if defined(__EMSCRIPTEN__) && defined(AE_USE_WEBGPU)
    if (GfxFactory::GetBackend() != GfxBackend::WebGPU)
#endif
        AssetManager::Get().LoadDefaultShaders();
    AssetManager::Get().LoadDefaultTextures();

    EnsureDefaultMaterial();

    InitializeLua();
    BindEngineAPIs();
    LoadUserScripts();
    CacheCallbacks();

    // GoScene must be called AFTER CacheCallbacks() so that _luaLoad is valid
    // when OnLoad() fires. On native, Prefetch is synchronous, meaning the
    // onReady callback (→ OnLoad → Lua load()) executes immediately inside GoScene.
    GoScene("main", [this] { OnLoad(); });
}

void LuaApplication::EnsureDefaultMaterial() {
    // Idempotent — CreateMaterial() hands back an existing material. Needed on
    // every scene (re)load, not just at init: ClearSceneAssets() drops every
    // material, this one included, and createCubeMesh() & co. pick it up when
    // it exists.
    MaterialProps defaultProps;
    defaultProps.diffuse = glm::vec3(1.0f);
    AssetManager::Get().CreateMaterial("Default", defaultProps);
}

void LuaApplication::OnLoad() {
    EnsureDefaultMaterial();

    ArmWatchdog(kLoadBudgetMs);
    if (_luaLoad.valid()) {
        CallLua(_luaLoad, "load");
    }
    _scriptLoaded = true;

    if (_sceneLoadedHandler) _sceneLoadedHandler();
}

void LuaApplication::OnUpdate(float dt, float time) {
    // Nothing runs before load() or after a callback raised (see IsLuaHalted()).
    if (!_scriptLoaded || IsLuaHalted()) return;

    // One budget covers update() plus every ScriptableComponent tick and
    // collision callback that follows it in this frame.
    ArmWatchdog(kFrameBudgetMs);

    // Call Lua update(dt) callback
    if (_luaUpdate.valid()) {
        CallLua(_luaUpdate, "update", dt);
    }

    // Handle keyboard events
    // TODO: Implement proper event system instead of polling
    // For now, we'll let Lua poll input via atmos.input.isKeyDown()
}

void LuaApplication::InitializeLua() {
    // Open standard Lua libraries.
    // On Emscripten sol::lib::io and sol::lib::os map to MEMFS; text files
    // written there by FileSystem::Prefetch() are accessible via io.open()
    // and the Lua require() system.
    _lua.open_libraries(
        sol::lib::base,
        sol::lib::package,
        sol::lib::string,
        sol::lib::math,
        sol::lib::table,
        sol::lib::io,
        sol::lib::os,
        sol::lib::debug
    );

    // Hooks are per-state (and inherited by coroutines), so this is redone for
    // every fresh state ReloadScript() creates.
    lua_sethook(_lua.lua_state(), &WatchdogHook, LUA_MASKCOUNT, kWatchdogInstructions);
    ArmWatchdog(kLoadBudgetMs);

    std::string packagePath = _lua["package"]["path"];
    // Add relative paths first to prioritize the current working directory
    packagePath += ";./assets/scripts/?.lua;./assets/scripts/?/init.lua";
    packagePath += ";./assets/?.lua;./assets/?/init.lua";
    packagePath += ";./?.lua;./?/init.lua";

    // Add resolved fallback paths next (only if the directory exists)
    if (auto baseScripts = FileSystem::Get().ResolvePath("assets/scripts")) {
        packagePath += ";" + *baseScripts + "/?.lua";
        packagePath += ";" + *baseScripts + "/?/init.lua";
    }
    _lua["package"]["path"] = packagePath;

    ENGINE_LOG("Lua environment initialized");
}

void LuaApplication::BindEngineAPIs() {
    // Create atmos namespace
    sol::table atmos = _lua.create_named_table("atmos");

    // Bind core types (vec2, vec3, etc.)
    BindCoreTypes(_lua);

    // Bind input API
    BindInputAPI(_lua, InputSubsystem::Get());

    // Bind World/Scene API
    BindWorldAPI(_lua, this);

    // Bind Graphics API
    BindGraphicsAPI(_lua, GraphicsSubsystem::Get());

    // Bind Physics API
    BindPhysicsAPI(_lua, this);

    // Bind Audio API
    BindAudioAPI(_lua, AudioSubsystem::Get());

    // Bind application-level functions
    atmos["quit"] = [this]() { RequestQuit(); };
    atmos["getTime"] = [this]() { return GetWindowTime(); };
    atmos["getDeltaTime"] = []() {
        // TODO: Store dt in a accessible place
        return 0.016f;
    };

    ENGINE_LOG("Engine APIs bound to Lua");
}

void LuaApplication::RequestQuit() {
    if (_quitHandler) {
        _quitHandler();
    } else {
        Quit();
    }
}

void LuaApplication::LoadUserScripts() {
    // Look for main.lua in assets/scripts/
    std::vector<std::string> scriptPaths = { "./assets/scripts/main.lua", "./assets/main.lua", "./main.lua" };

    bool loaded = false;
    for (const auto& path : scriptPaths) {
        std::string targetPath = path;
        bool exists = false;

#ifndef __EMSCRIPTEN__
        // On native platforms, prioritize files in the current working directory
        if (std::filesystem::exists(path)) {
            targetPath = path;
            exists = true;
        } else
#endif
        {
            // Fall back to resolved virtual paths (cache, MEMFS, or g_basePath on native)
            if (auto resolved = FileSystem::Get().ResolvePath(path)) {
                targetPath = *resolved;
                exists = true;
            }
        }

        if (exists) {
            auto result = _lua.safe_script_file(targetPath, sol::script_pass_on_error);
            if (!result.valid()) {
                HandleError(result, "Loading " + targetPath);
            } else {
                ENGINE_LOG("Loaded script: {}", targetPath);

                // Dynamically update package.path to prioritize the loaded script's directory
                std::filesystem::path p(targetPath);
                std::string baseDir = p.parent_path().string();
                if (baseDir.empty()) {
                    baseDir = ".";
                }
                std::string packagePath = _lua["package"]["path"];
                packagePath = baseDir + "/?.lua;" + baseDir + "/?/init.lua;" + packagePath;
                _lua["package"]["path"] = packagePath;
                ENGINE_LOG("Updated Lua package.path to prioritize: {}", baseDir);

                loaded = true;
                break;
            }
        }
    }

    if (!loaded) {
        ENGINE_LOG("Warning: No main.lua found. Create one in assets/scripts/main.lua");
    }
}

void LuaApplication::CacheCallbacks() {
    // Cache global callbacks for efficient access
    auto tryCache = [this](const char* name) -> sol::protected_function {
        sol::object obj = _lua[name];
        if (obj.is<sol::protected_function>()) {
            return obj.as<sol::protected_function>();
        }
        return sol::lua_nil;
    };

    _luaLoad = tryCache("load");
    _luaUpdate = tryCache("update");
    _luaDraw = tryCache("draw");
    _luaKeypressed = tryCache("keypressed");
    _luaKeyreleased = tryCache("keyreleased");

    if (_luaLoad.valid()) ENGINE_LOG("Found load() callback");
    if (_luaUpdate.valid()) ENGINE_LOG("Found update() callback");
    if (_luaDraw.valid()) ENGINE_LOG("Found draw() callback");
}

void LuaApplication::ResetCallbacks() {
    // sol unrefs on assignment, so this has to happen while the state that
    // owns the references is still alive.
    _luaLoad = sol::lua_nil;
    _luaUpdate = sol::lua_nil;
    _luaDraw = sol::lua_nil;
    _luaKeypressed = sol::lua_nil;
    _luaKeyreleased = sol::lua_nil;
}

bool LuaApplication::CheckSyntax(const std::string& source, const std::string& chunkName) {
    // load() only compiles; the resulting function is dropped with `chunk`.
    sol::load_result chunk = _lua.load(source, chunkName);
    if (chunk.valid()) return true;

    sol::error err = chunk;
    ReportLuaError("Compiling " + DisplayName(chunkName), err.what());
    return false;
}

bool LuaApplication::RunChunk(const std::string& source, const std::string& chunkName) {
    auto result = _lua.safe_script(source, sol::script_pass_on_error, chunkName);
    if (!result.valid()) {
        HandleError(result, "Running " + DisplayName(chunkName));
        return false;
    }
    return true;
}

void LuaApplication::DestroyScriptEntities() {
    // atmos.world.spawn() creates top-level GameObjects (no parent), which the
    // scene unload never reaches — it only walks the containers under __root__.
    // Collect the roots first; DestroyGameObject() takes each subtree with it.
    GameObject* sceneRoot = GetDefaultGameObject();
    std::vector<GameObject*> roots;
    for (const auto& entity : GetEntities()) {
        if (entity.get() != sceneRoot && entity->parent == nullptr) roots.push_back(entity.get());
    }
    for (auto* go : roots) {
        DestroyGameObject(go);
    }
}

bool LuaApplication::ReloadScript(const std::string& source, const std::string& chunkName) {
    ArmWatchdog(kLoadBudgetMs);// covers onDestroy() during teardown and the chunk itself
    if (!CheckSyntax(source, chunkName)) return false;

    ENGINE_LOG("Reloading script {}", DisplayName(chunkName));

    // 1. Everything that references the current lua_State goes first, in
    //    dependency order: cached callbacks, script-spawned GameObjects (their
    //    ScriptableComponents hold sol references), then the scene itself —
    //    synchronously, so no frame renders between the two steps with render
    //    lists pointing at freed objects.
    ResetCallbacks();
    _scriptLoaded = false;
    SetLuaHalted(false);
    DestroyScriptEntities();
    UnloadCurrentScene();

    // 2. A fresh state: no stale globals, no leaked classes from the last run.
    _lua = sol::state{};
    InitializeLua();
    BindEngineAPIs();

    // 3. Run the chunk, then rebuild the scene and call load() once more. A
    //    failed chunk still goes through GoScene so the scene ends up in a known
    //    state (empty, halted) rather than half torn down.
    const bool ok = RunChunk(source, chunkName);
    CacheCallbacks();
    GoScene("main", [this] { OnLoad(); });
    return ok;
}

void LuaApplication::HandleError(const sol::protected_function_result& result, const std::string& context) {
    sol::error err = result;

    // Stop every callback until the next reload: a broken update() would
    // otherwise raise the same error again every frame.
    SetLuaHalted(true);
    ReportLuaError(context, err.what());
}

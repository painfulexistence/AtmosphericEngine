# Lua Scripting System Redesign Proposal

## Overview

This document proposes a redesign of the Lua scripting system to support a hybrid architecture where **C++ remains the primary development language** while **Lua serves as a rapid prototyping tool**.

## Current Problems

### 1. Architectural Issues

| Problem | Current State | Impact |
|---------|--------------|--------|
| Script as Server | `Script` inherits from `Server`, treated as a subsystem | Wrong abstraction - scripting is a control layer, not a service |
| Initialization Timing | `init()` called before `OnLoad()` | Lua cannot access scene objects in `init()` |
| Dual Control | Both C++ `OnUpdate()` and Lua `update()` exist | Unclear responsibility boundaries |
| No Draw Callback | Lua has no way to intervene in rendering | Cannot prototype visual effects in Lua |
| Hardcoded Member | `Script script;` is a fixed member of Application | Not optional, cannot be replaced |

### 2. Implementation Issues

| Problem | Location | Impact |
|---------|----------|--------|
| Empty `Bind()` function | `script.cpp:45-49` | No C++ APIs exposed to Lua |
| String concatenation execution | `script.cpp:42` | Performance overhead, potential injection risk |
| Silent error handling | `script.cpp:61-69` | Debugging impossible |
| Unused `ScriptData` | `component.hpp:53-55` | Dead code |

## Proposed Architecture

### High-Level Design

```
┌─────────────────────────────────────────────────────┐
│                  Game Logic Layer                    │
│  ┌─────────────────┐    ┌─────────────────────┐     │
│  │   C++ Code      │ OR │   Lua Script        │     │
│  │  (Production)   │    │  (Prototyping)      │     │
│  └────────┬────────┘    └──────────┬──────────┘     │
│           │                        │                 │
│           └──────────┬─────────────┘                 │
│                      ▼                               │
├─────────────────────────────────────────────────────┤
│               Scripting Bridge                       │
│         (Unified API for both C++ and Lua)          │
├─────────────────────────────────────────────────────┤
│   Graphics    Physics    Audio    Input    ...      │
│                   (Services Layer)                   │
└─────────────────────────────────────────────────────┘
```

### Core Components

#### 1. ScriptRuntime (replaces Script)

**Key Change**: Does NOT inherit from `Server`. It's a standalone runtime manager.

```cpp
class ScriptRuntime {
public:
    static ScriptRuntime& Get();

    // Lifecycle
    void Initialize();           // Setup environment only
    void Shutdown();

    // Script loading
    void LoadScript(const std::string& path);
    void ReloadScript(const std::string& path);  // Hot reload
    void ReloadAll();

    // Callbacks (called by Application at appropriate times)
    void OnLoad();               // After scene is ready
    void OnUpdate(float dt);
    void OnDraw();
    void OnUnload();

    // Event callbacks
    void OnKeyPressed(int key);
    void OnKeyReleased(int key);
    void OnMousePressed(int button, float x, float y);
    void OnMouseReleased(int button, float x, float y);
    void OnMouseMoved(float x, float y);

    // Direct Lua access
    sol::state& GetState();

    // Call Lua functions from C++
    template<typename R, typename... Args>
    std::optional<R> Call(const std::string& func, Args&&... args);

private:
    sol::state _lua;
    std::vector<std::string> _loadedScripts;

    // Cached callbacks for performance
    sol::protected_function _onLoad;
    sol::protected_function _onUpdate;
    sol::protected_function _onDraw;
    sol::protected_function _onUnload;

    void CacheCallbacks();
    void HandleError(const sol::error& err);
};
```

#### 2. ScriptableComponent

Allows Lua to define component behaviors, similar to Unity's MonoBehaviour.

```cpp
class ScriptableComponent : public Component {
public:
    ScriptableComponent(GameObject* owner, const std::string& scriptClass);

    std::string GetName() const override;

    void OnAttach() override;
    void OnDetach() override;
    void OnTick(float dt) override;

    // Access Lua instance data
    sol::table& GetSelf();

    // Call methods on the Lua instance
    template<typename... Args>
    void CallMethod(const std::string& method, Args&&... args);

private:
    std::string _scriptClass;
    sol::table _self;

    // Cached method references
    sol::protected_function _onAttach;
    sol::protected_function _onDetach;
    sol::protected_function _onUpdate;
};
```

#### 3. API Binding Modules

Organized by domain for maintainability:

```
src/scripting/
├── script_runtime.cpp
├── scriptable_component.cpp
└── bindings/
    ├── bind_core.cpp      # vec3, mat4, GameObject, Component
    ├── bind_input.cpp     # Input API
    ├── bind_graphics.cpp  # Graphics API
    ├── bind_audio.cpp     # Audio API
    ├── bind_physics.cpp   # Physics API
    └── bind_world.cpp     # Scene/World management
```

## Initialization Flow

### Current (Problematic)

```
script.Init()
├── open_libraries()
├── Source("config.lua")
├── Source("main.lua")
└── Run("init()")        ← Scene not loaded yet!
OnInit()
OnLoad()                 ← Scene loads here
MainLoop
```

### Proposed

```
Phase 1: Service Initialization
├── graphics.Init()
├── physics.Init()
├── audio.Init()
└── input.Init()

Phase 2: Script Runtime Setup
├── ScriptRuntime::Initialize()
└── ScriptRuntime::BindAPIs()    ← Only bind, don't execute

Phase 3: C++ Initialization
└── OnInit()

Phase 4: Load Scripts
└── ScriptRuntime::LoadScript("main.lua")  ← Parse only

Phase 5: Scene Loading
└── OnLoad()             ← C++ loads scene

Phase 6: Lua Ready Callback
└── ScriptRuntime::OnLoad()  ← Now Lua can access scene!

Phase 7: Main Loop
└── while (running)
    ├── input.Poll()
    ├── OnUpdate(dt)
    ├── ScriptRuntime::OnUpdate(dt)
    ├── physics.Step(dt)
    ├── ScriptRuntime::OnDraw()
    └── graphics.Present()

Phase 8: Shutdown
└── ScriptRuntime::OnUnload()
```

## API Design

### Lua-side API (Love2D-inspired)

```lua
-- Global callbacks
function load()
    -- Called after scene is ready
end

function update(dt)
    -- Called every frame
end

function draw()
    -- Called every frame for rendering
end

function keypressed(key)
    -- Called on key press
end

-- Namespaced APIs
atmos.graphics.drawSprite(sprite, x, y)
atmos.graphics.drawText(text, x, y)
atmos.graphics.setColor(r, g, b, a)

atmos.input.isKeyDown(key)
atmos.input.getMousePosition()

atmos.audio.play(sound)
atmos.audio.setVolume(volume)

atmos.physics.raycast(from, to)

atmos.world.spawn(prefab, x, y, z)
atmos.world.destroy(entity)
atmos.world.find(name)
```

### Component Classes in Lua

```lua
-- Define a component class
PlayerController = {}
PlayerController.__index = PlayerController

function PlayerController:new(gameObject)
    local self = setmetatable({}, PlayerController)
    self.gameObject = gameObject
    self.speed = 5.0
    return self
end

function PlayerController:onAttach()
    print("Attached to " .. self.gameObject.name)
end

function PlayerController:onUpdate(dt)
    -- Component logic here
end

-- Usage
local player = atmos.world.spawn("player", 0, 0, 0)
player:addScript("PlayerController")
```

## Hot Reload Support

### Design

```cpp
class ScriptRuntime {
public:
    // Reload a single script
    void ReloadScript(const std::string& path) {
        // 1. Store current state of ScriptableComponents
        auto states = SaveComponentStates();

        // 2. Clear old definitions (but keep data)
        ClearScriptDefinitions(path);

        // 3. Reload the script file
        LoadScript(path);

        // 4. Restore component states
        RestoreComponentStates(states);

        // 5. Call reload callback if defined
        if (auto onReload = _lua["onReload"]; onReload.valid()) {
            onReload();
        }
    }

    // File watcher integration
    void EnableHotReload(bool enabled);
    void CheckForChanges();  // Call in update loop

private:
    FileWatcher _watcher;
    std::unordered_map<std::string, std::filesystem::file_time_type> _fileTimestamps;
};
```

### What Gets Preserved on Reload

| Preserved | Not Preserved |
|-----------|---------------|
| Component instance data (self table) | Function definitions |
| Global variables | Class metatables |
| GameObject references | Cached function references |

## Migration Path

### From Lua Prototype to C++ Production

```lua
-- Lua prototype
PlayerController = {}
function PlayerController:onUpdate(dt)
    local move = vec3(0, 0, 0)
    if atmos.input.isKeyDown(KEY_W) then move.z = -1 end
    if atmos.input.isKeyDown(KEY_S) then move.z = 1 end
    self.gameObject.position = self.gameObject.position + move * self.speed * dt
end
```

```cpp
// C++ production version (same logic, native performance)
class PlayerController : public Component {
    void OnTick(float dt) override {
        glm::vec3 move(0.0f);
        if (Input::Get()->GetKeyDown(KEY_W)) move.z = -1;
        if (Input::Get()->GetKeyDown(KEY_S)) move.z = 1;
        gameObject->SetPosition(gameObject->GetPosition() + move * _speed * dt);
    }
};
```

## Implementation Phases

### Phase 1: Foundation ✅ COMPLETED
- [x] Create `LuaApplication` class (Application subclass pattern)
- [x] Fix initialization order (load() called after scene ready)
- [x] Implement proper error handling with stack traces
- [x] Basic callbacks: `load()`, `update(dt)`, `draw()`

### Phase 2: Core Bindings ✅ COMPLETED
- [x] Math types: `vec3`, `vec2`, `vec4`, `quat`
- [x] `GameObject` usertype with position/rotation/scale
- [x] Input API: `isKeyDown`, `isKeyPressed`, `getMousePosition`
- [x] Basic world API: `spawn`, `destroy`, `find`
- [x] Graphics API: `getMainCamera`, `getMainLight`, `reloadShaders`
- [x] Asset management: `createCubeMesh`, `createSphereMesh`, `getMesh`

### Phase 3: ScriptableComponent ✅ COMPLETED
- [x] Implement `ScriptableComponent` class
- [x] Lua class instantiation (with `new()` constructor support)
- [x] Lifecycle callbacks: `init()`, `update(dt)`, `physicsUpdate(dt)`, `onCollision()`, `onDestroy()`
- [x] Method caching for performance
- [x] `gameObject:addScript("ClassName")` binding
- [x] Access to instance table via `getScript().instance`

### Phase 4: Extended APIs ✅ COMPLETE
- [x] Graphics API: `drawSprite`, `drawRect`, `setColor`, `begin2D`/`end2D`
- [x] Audio API: `loadSound`, `playSound`, `loadMusic`, `playMusic`, volume control
- [x] Text rendering: `loadFont`, `drawText`, `measureText` (via stb_truetype)
- [x] Physics API: `raycast`, `addForce`, `addImpulse`, `setGravity`
- [x] RigidbodyComponent bindings: velocity, forces, sleep/wake
- [x] Collision callbacks: `onCollision(other)` in ScriptableComponent

### Phase 5: Hot Reload (Nice to have)
- [ ] File change detection
- [ ] Script reload without restart
- [ ] State preservation across reloads

### Phase 6: Developer Experience (Nice to have)
- [ ] Lua error overlay (like Love2D's blue screen)
- [ ] Console integration for live Lua execution
- [ ] Script profiling

## File Structure

**Current Implementation:**

```
AtmosphericEngine/
├── AtmosphericEngine/           # Core engine library
│   └── include/Atmospheric/     # Public headers
├── frontends/
│   └── lua/                     # Lua frontend (AtmosLua.exe)
│       ├── main.cpp             # Entry point
│       ├── lua_application.hpp  # LuaApplication class
│       ├── lua_application.cpp
│       ├── scriptable_component.hpp  # ScriptableComponent
│       ├── scriptable_component.cpp
│       ├── example_main.lua     # Example game script
│       ├── CMakeLists.txt
│       └── bindings/
│           ├── bind_core.cpp     # vec2, vec3, vec4, quat
│           ├── bind_input.cpp    # Input API + key constants
│           ├── bind_graphics.cpp # Graphics, Camera, Light, 2D Drawing
│           ├── bind_world.cpp    # GameObject, World, Scene APIs
│           ├── bind_physics.cpp  # Raycast, RigidbodyComponent, Forces
│           └── bind_audio.cpp    # Sound, Music, Volume control
└── docs/
    └── lua-scripting-proposal.md  # This document
```

**Runtime Output:**

```
build/bin/AtmosLua/
├── AtmosLua.exe
└── assets/
    ├── scripts/
    │   └── main.lua             # User's game script
    ├── shaders/
    ├── textures/
    └── scenes/
```

## Data-Driven Architecture Under Scripting

### The Problem: FFI Overhead

When Lua spawns entities by calling C++ APIs repeatedly, each call crosses the language boundary:

```lua
-- Naive approach: 4+ FFI calls per entity
function load()
    for i = 1, 1000 do
        local e = atmos.world.spawn()      -- 1. Lua→C++ create GameObject
        e:addMesh("enemy")                  -- 2. Lua→C++ load mesh, register renderer
        e:addRigidbody({mass = 1})          -- 3. Lua→C++ create physics body
        e:addScript("EnemyAI")              -- 4. Lua→C++ create component
    end
end
-- 1000 entities × 4 calls = 4000+ FFI crossings
```

This is inefficient because:
1. Each FFI call has overhead (~100-500ns)
2. No batching opportunity
3. Memory allocation scattered across calls

### Why Love2D Doesn't Have This Problem

Love2D's "entities" are just Lua tables:

```lua
-- Love2D style: zero FFI calls for "spawning"
function love.load()
    enemies = {}
    for i = 1, 1000 do
        table.insert(enemies, {
            x = math.random(800),
            y = math.random(600),
            sprite = enemyImage,  -- Pre-loaded image reference
            health = 100
        })
    end
end

function love.draw()
    for _, e in ipairs(enemies) do
        love.graphics.draw(e.sprite, e.x, e.y)  -- Only here: Lua→C++
    end
end
```

Love2D has no GameObject/Component system. "Entities" are pure Lua data with zero C++ involvement until rendering.

### Solution: Shared C++ Scene Loader

Instead of Lua calling spawn APIs, let C++ batch-process scene data:

```
┌──────────────────────────────────────────────────────────────┐
│                     Configuration Layer                       │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐           │
│  │ level1.lua  │  │ level2.lua  │  │ prefabs.lua │           │
│  │ (scene def) │  │ (scene def) │  │ (templates) │           │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘           │
└─────────┼────────────────┼────────────────┼──────────────────┘
          │                │                │
          └────────────────┼────────────────┘
                           ▼
┌──────────────────────────────────────────────────────────────┐
│                      C++ SceneLoader                          │
│  - Single FFI call from Lua                                   │
│  - Batch creates all GameObjects                              │
│  - Batch attaches all Components                              │
│  - Can optimize memory layout                                 │
└─────────────────────────┬────────────────────────────────────┘
                          │
          ┌───────────────┼───────────────┐
          ▼               ▼               ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│ C++ OnLoad() │  │ Lua load()   │  │ Runtime      │
│ Direct call  │  │ via binding  │  │ spawn()      │
└──────────────┘  └──────────────┘  └──────────────┘
```

### Scene Configuration Format

```lua
-- assets/scenes/level1.lua
return {
    entities = {
        player = {
            position = {0, 1, 0},
            rotation = {0, 0, 0},
            components = {
                mesh = { name = "player_mesh" },
                rigidbody = { mass = 1.0, kinematic = false },
                script = { class = "PlayerController" }
            }
        },
        main_camera = {
            position = {0, 5, -10},
            components = {
                camera = { fov = 60, near = 0.1, far = 1000 }
            }
        }
    },

    -- Batch spawning
    spawners = {
        {
            prefab = "enemy",
            count = 100,
            area = {-50, -50, 50, 50},
            random_rotation = true
        },
        {
            prefab = "tree",
            count = 500,
            area = {-100, -100, 100, 100}
        }
    }
}
```

### C++ SceneLoader Implementation

```cpp
class SceneLoader {
public:
    // Load from Lua table - called once, creates everything
    static void LoadFromTable(sol::table sceneData) {
        auto& app = *Application::Get();

        // Batch create entities
        if (auto entities = sceneData["entities"]; entities.valid()) {
            for (auto& [name, data] : entities) {
                CreateEntity(name, data);
            }
        }

        // Batch spawners
        if (auto spawners = sceneData["spawners"]; spawners.valid()) {
            for (auto& spawner : spawners) {
                ProcessSpawner(spawner);
            }
        }
    }

    // Load from file path
    static void LoadFromFile(const std::string& name) {
        auto& lua = ScriptRuntime::Get().GetState();
        std::string path = "assets/scenes/" + name + ".lua";

        sol::table sceneData = lua.script_file(path);
        LoadFromTable(sceneData);
    }

private:
    static void CreateEntity(const std::string& name, sol::table data) {
        auto* go = Application::Get()->CreateGameObject();
        go->SetName(name);

        // Position/rotation/scale
        if (auto pos = data["position"]; pos.valid()) {
            go->SetPosition(TableToVec3(pos));
        }
        if (auto rot = data["rotation"]; rot.valid()) {
            go->SetRotation(TableToVec3(rot));
        }

        // Components - all attached in C++, no FFI per component
        if (auto components = data["components"]; components.valid()) {
            AttachComponents(go, components);
        }
    }

    static void ProcessSpawner(sol::table spawner) {
        std::string prefab = spawner["prefab"];
        int count = spawner["count"];
        sol::table area = spawner["area"];

        // Batch create - much faster than individual Lua calls
        for (int i = 0; i < count; i++) {
            auto* go = InstantiatePrefab(prefab);
            go->SetPosition(RandomPositionInArea(area));
        }
    }
};
```

### Binding for Lua Access

```cpp
void BindSceneAPI(sol::state& lua) {
    sol::table scene = lua["atmos"]["scene"];

    // Single call loads entire scene
    scene["load"] = [](const std::string& name) {
        SceneLoader::LoadFromFile(name);
    };

    // Load from inline table
    scene["loadTable"] = [](sol::table data) {
        SceneLoader::LoadFromTable(data);
    };

    // Runtime spawning (for dynamic objects)
    scene["spawn"] = [](const std::string& prefab, float x, float y, float z) {
        return SceneLoader::InstantiatePrefab(prefab, {x, y, z});
    };
}
```

### Usage Comparison

**C++ Production Code:**
```cpp
void MyGame::OnLoad() {
    SceneLoader::LoadFromFile("level1");

    // Get handles for runtime manipulation
    player = FindGameObject("player");
    mainCamera = FindGameObject("main_camera");
}
```

**Lua Prototyping Code:**
```lua
function load()
    atmos.scene.load("level1")  -- Same loader, same result

    -- Get handles
    player = atmos.world.find("player")
    camera = atmos.world.find("main_camera")

    -- Runtime spawning still available for dynamic objects
    local pickup = atmos.scene.spawn("coin", 10, 1, 5)
end
```

### Performance Comparison

| Approach | 1000 Entities | FFI Calls | Time |
|----------|---------------|-----------|------|
| Lua manual spawn | 4000+ | ~4000 | ~50ms |
| C++ SceneLoader | 1 | 1 | ~5ms |

### Key Design Principles

1. **Configuration in Lua, Execution in C++**
   - Scene data defined as Lua tables (human-readable, easy to edit)
   - Parsing and instantiation done in C++ (fast, batched)

2. **Single Entry Point**
   - One `scene.load()` call replaces thousands of individual API calls
   - C++ can optimize memory allocation, batch physics registration, etc.

3. **Same Loader for Both Languages**
   - C++ calls `SceneLoader::LoadFromFile()` directly
   - Lua calls `atmos.scene.load()` which wraps the same function
   - Consistent behavior, single source of truth

4. **Runtime Spawning as Exception**
   - Use `spawn()` for truly dynamic objects (projectiles, pickups)
   - Static level geometry should always go through scene files

## Appendix: Sol2 Binding Patterns

### Function Binding

```cpp
// Simple function
lua["add"] = [](int a, int b) { return a + b; };

// Member function
lua["getTime"] = &Application::GetWindowTime;

// With instance
lua["quit"] = [app]() { app->Quit(); };
```

### Usertype Binding

```cpp
lua.new_usertype<GameObject>("GameObject",
    // Constructors (usually not exposed)
    sol::no_constructor,

    // Properties
    "position", sol::property(&GameObject::GetPosition, &GameObject::SetPosition),
    "name", sol::property(&GameObject::GetName, &GameObject::SetName),

    // Read-only property
    "id", sol::readonly(&GameObject::id),

    // Methods
    "addComponent", &GameObject::AddComponent<ScriptableComponent>,

    // Operators
    sol::meta_function::to_string, [](GameObject* o) { return o->GetName(); }
);
```

### Namespace Organization

```cpp
sol::table atmos = lua.create_named_table("atmos");
sol::table graphics = atmos.create("graphics");
graphics["draw"] = &GraphicsAPI::Draw;
```

## Appendix: Scripting Language Comparison

### Architecture: ScriptApplication Pattern

The recommended architecture is to implement each scripting language as an `Application` subclass:

```
AtmosphericEngine (core library)
        │
        ├── libatmos.a / atmos.dll
        │
        ▼
┌───────────────────────────────────────────────────┐
│              Different "Frontends"                 │
├─────────────────┬─────────────────┬───────────────┤
│ LuaApp.exe      │ MonoApp.exe     │ MyGame.exe    │
│ (Love2D style)  │ (Unity style)   │ (Pure C++)    │
│                 │                 │               │
│ class LuaApp    │ class MonoApp   │ class MyGame  │
│ : Application   │ : Application   │ : Application │
└─────────────────┴─────────────────┴───────────────┘
```

**Benefits:**
- No abstraction layer needed (no `IScriptRuntime` interface)
- Each language frontend is completely independent
- Zero overhead from polymorphism
- Adding a new language = adding a new Application subclass

### Language Comparison

| Feature | Lua (Sol2) | Luau (LuaBridge3) | Mono (C#) | Lobster |
|---------|------------|-------------------|-----------|---------|
| **Type System** | Dynamic | Gradual (optional) | Static | Static |
| **JIT** | LuaJIT ✅ | ✅ (weaker) | ✅ | ❌ |
| **AOT** | ❌ | ❌ | ✅ | ✅ |
| **GC** | Yes | Yes | Yes | No (compile-time) |
| **Binding Convenience** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ |
| **IDE Support** | Basic | Basic | Excellent | Basic |
| **Runtime Size** | ~500KB | ~500KB | ~20-50MB | ~2MB |
| **Community** | Large | Medium | Large | Small |
| **Embedding Design** | Native | Native | Native | Standalone-first |

### Binding Code Comparison

**Lua (Sol2) - Most concise:**
```cpp
lua.new_usertype<GameObject>("GameObject",
    "position", sol::property(&GameObject::GetPosition, &GameObject::SetPosition),
    "addMesh", &GameObject::AddMesh
);
```

**Luau (LuaBridge3) - Similar:**
```cpp
luabridge::getGlobalNamespace(L)
    .beginClass<GameObject>("GameObject")
        .addProperty("position", &GameObject::GetPosition, &GameObject::SetPosition)
        .addFunction("addMesh", &GameObject::AddMesh)
    .endClass();
```

**Mono (C#) - Requires both sides:**
```cpp
// C++ side
mono_add_internal_call("Atmos.GameObject::GetPosition", (void*)GameObject_GetPosition);

// C# side (also required)
public class GameObject {
    [MethodImpl(MethodImplOptions.InternalCall)]
    public extern Vector3 GetPosition();
}
```

### Estimated Implementation Cost

| Component | Lua | Luau | Mono |
|-----------|-----|------|------|
| Application subclass | ~200 lines | ~200 lines | ~400 lines |
| 100 API bindings | ~400 lines | ~500 lines | ~1400 lines |
| Hot reload | ~100 lines | ~100 lines | ~500 lines |
| **Total** | **~700 lines** | **~800 lines** | **~2300 lines** |

### Recommendation

| Use Case | Recommended |
|----------|-------------|
| Rapid prototyping, small teams | **Lua** — simplest binding |
| Type safety without AOT | **Luau** — gradual typing |
| Large projects, Unity familiarity | **Mono** — full static typing + IDE |
| Minimal runtime, AOT required | Consider **AngelScript** or **Mono AOT** |

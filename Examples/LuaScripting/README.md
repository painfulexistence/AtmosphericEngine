# LuaScripting

A Löve2D-style Lua frontend for the engine: `assets/scripts/main.lua` drives
the game through `load()` / `update(dt)`, and Lua classes become
`ScriptableComponent`s on GameObjects (`go:addScript("Spinner")`). The frontend
itself lives in `Engine/frontends/lua/`; this example is its runtime plus a
sample script.

On the web it ships as a **playground**: the game on the left, a live code
editor on the right.

## Web playground

```bash
./scripts/buildWasm.sh release          # Release builds use shell.html (the playground)
# → http://localhost:8000/LuaScripting/LuaScripting.html
```

- **Run** (`Ctrl+Enter` / `⌘+Enter`) rebuilds the scene from the editor: every
  GameObject the previous run spawned is destroyed, the Lua state is recreated,
  the chunk runs, the scene reloads and `load()` is called again.
- **Reset** restores the original `main.lua` and runs it.
- `print()` and script errors land in the console panel under the editor.
- Edits are kept as a draft in `localStorage`; reload the page and they come
  back (Reset discards the draft).
- **Escape** in the game view calls `atmos.quit()`, which the playground turns
  into a scene restart.
- Layout: drag the divider to resize; on narrow screens the panes stack.

### What happens with broken code

| Problem | Behaviour |
|---------|-----------|
| Syntax error | Reported immediately with the line marked in the editor; the running scene is left untouched. |
| Runtime error in `load()`, `update()` or a component | Every Lua callback stops (the scene freezes on its last frame), the error is shown over the game view and the line is marked. Press Run again after fixing it. |
| Infinite loop | A Lua count hook interrupts any callback that runs longer than 3 s in one frame (15 s for `load()`), reporting it like any other error. |

### How the pieces fit

| Piece | Role |
|-------|------|
| `shell.html` | The page: layout, console panel, error banner, editor iframe, fullscreen, the `Module.onLua*` hooks. Used for Release builds only (`--shell-file`). |
| `playground/editor.html` | The editor, in its own iframe so keystrokes never reach the game's GLFW key listeners. CodeMirror 5 is vendored under `playground/vendor/`. |
| `Engine/frontends/lua/runtime.cpp` | Web bridge: `ae_lua_run(source, len)` compiles the chunk (syntax errors return synchronously) and queues the reload for the next frame via `DeferSpawn`; forwards errors / scene-loaded / quit to the page. |
| `LuaApplication::ReloadScript` | The reload itself, plus the watchdog hook and the "halt after an error" rule (`lua_diagnostics.hpp`). |

## Native

```bash
cmake --preset=dev && cmake --build --preset=dev
./build/dev/LuaScripting/LuaScripting     # runs ./assets/scripts/main.lua
```

There is no editor natively; edit the file and restart. Errors go to stderr,
`atmos.quit()` closes the window.

## Script API cheat sheet

```lua
local go = atmos.world.spawn(x, y, z)         -- GameObject
go.name, go.position, go.rotation, go.scale   -- vec3 in/out (copy, assign back)
go:addMesh("Cube")                            -- after atmos.assets.createCubeMesh("Cube", 1.0)
go:addLight({ type = 0, intensity = 1, castShadow = true })
go:addCamera({ fov = 60, near = 0.1, far = 100 })
go:addRigidbody({ mass = 1, kinematic = false, gravity = true })
go:addScript("ClassName")                     -- Lua class with init/update/onCollision/...
go:getScript().instance                       -- the Lua table behind the first script

atmos.input.isKeyDown / isKeyPressed / isKeyReleased(atmos.keys.W)
atmos.physics.raycast(fromVec3, toVec3)      -- table { point, normal, distance, gameObject } or nil
atmos.audio.loadSound / playSound / playMusic ...
atmos.graphics.reloadShaders(), drawText / drawRect / ... (immediate mode 2D)
atmos.quit()
```

See `Engine/frontends/lua/bindings/` for the full surface.

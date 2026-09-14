-- =============================================================================
-- Atmospheric Lua Playground - main.lua
--
-- Edit this file in the panel on the right and press Run (Ctrl+Enter). The
-- scene is rebuilt from scratch on every run: everything the previous run
-- spawned is gone, the Lua state is fresh, and load() is called again.
--
-- The engine calls two global functions:
--   load()      once the scene is ready
--   update(dt)  every frame, dt in seconds
--
-- Controls (click the game view first):
--   WASD   move the ball          SPACE  spin the cube faster
--   R      reload shaders         ESC    restart the scene (quits natively)
-- =============================================================================

local ball
local cube
local time = 0

-- =============================================================================
-- ScriptableComponent: Spinner - rotates its GameObject around an axis.
--
-- A component is a plain Lua class attached with go:addScript("Spinner").
-- The engine fills in self.gameObject and looks for these methods:
--   init(), update(dt), physicsUpdate(dt), onCollision(other), onDestroy()
-- =============================================================================
Spinner = {}
Spinner.__index = Spinner

function Spinner:new()
    local self = setmetatable({}, Spinner)
    self.speed = 1.0
    self.axis = vec3.new(0, 1, 0)  -- Rotate around Y axis by default
    return self
end

function Spinner:init()
    print("[Spinner] attached to " .. tostring(self.gameObject))
end

function Spinner:update(dt)
    local go = self.gameObject
    local rot = go.rotation
    rot.y = rot.y + self.speed * dt
    go.rotation = rot
end

-- =============================================================================
-- ScriptableComponent: Bobber - moves its GameObject up and down.
-- =============================================================================
Bobber = {}
Bobber.__index = Bobber

function Bobber:new()
    local self = setmetatable({}, Bobber)
    self.amplitude = 0.5
    self.frequency = 1.0
    self.time = 0
    self.baseY = 0
    return self
end

function Bobber:init()
    -- Remember where we started so the bob is relative to it.
    self.baseY = self.gameObject.position.y
end

function Bobber:update(dt)
    self.time = self.time + dt
    local pos = self.gameObject.position
    pos.y = self.baseY + math.sin(self.time * self.frequency * math.pi * 2) * self.amplitude
    self.gameObject.position = pos
end

-- =============================================================================
-- Game
-- =============================================================================
function load()
    print("[Lua] load()")

    -- A directional light (type 0), angled down.
    local sun = atmos.world.spawn(0, 10, 0)
    sun.name = "Sun"
    sun.rotation = vec3.new(math.rad(45), math.rad(45), 0)
    sun:addLight({
        type = 0,
        intensity = 1.0,
        castShadow = true
    })

    -- Meshes are created by name, then attached by name.
    atmos.assets.createCubeMesh("Cube", 1.0)
    atmos.assets.createSphereMesh("Ball", 0.5, 24)

    -- The cube spins and bobs through two ScriptableComponents.
    cube = atmos.world.spawn(5, 0, 0)
    cube.name = "Cube"
    cube.rotation = vec3.new(0, 0, math.rad(45))
    cube:addMesh("Cube")
    cube:addScript("Spinner")
    cube:addScript("Bobber")

    -- getScript() returns the first component attached (the Spinner);
    -- .instance is the Lua table behind it, so its fields are yours to tweak.
    local spinner = cube:getScript()
    if spinner then
        spinner.instance.speed = 5.0
    end

    -- The ball you drive around with WASD.
    ball = atmos.world.spawn(0, 1, -5)
    ball.name = "Ball"
    ball:addMesh("Ball")

    print("[Lua] Ready. WASD moves the ball, SPACE spins the cube faster, ESC restarts.")
end

function update(dt)
    time = time + dt

    if ball then
        local speed = 5.0
        local pos = ball.position
        local moved = false

        if atmos.input.isKeyDown(atmos.keys.W) then
            pos.z = pos.z + speed * dt
            moved = true
        end
        if atmos.input.isKeyDown(atmos.keys.S) then
            pos.z = pos.z - speed * dt
            moved = true
        end
        if atmos.input.isKeyDown(atmos.keys.A) then
            pos.x = pos.x - speed * dt
            moved = true
        end
        if atmos.input.isKeyDown(atmos.keys.D) then
            pos.x = pos.x + speed * dt
            moved = true
        end

        if moved then
            ball.position = pos
        end
    end

    -- SPACE: spin the cube faster (tap it a few times).
    if cube and atmos.input.isKeyPressed(atmos.keys.SPACE) then
        local spinner = cube:getScript()
        if spinner then
            spinner.instance.speed = math.min(spinner.instance.speed * 1.5, 40)
            print(string.format("[Lua] cube spin speed: %.1f", spinner.instance.speed))
        end
    end

    -- R: reload shaders from disk.
    if atmos.input.isKeyPressed(atmos.keys.R) then
        print("[Lua] reloading shaders")
        atmos.graphics.reloadShaders()
    end

    -- ESC: restarts the scene in the web playground, quits the native runtime.
    if atmos.input.isKeyPressed(atmos.keys.ESCAPE) then
        atmos.quit()
    end
end

function draw()
    -- Optional: custom draw logic. Rendering is handled by the C++ engine.
end

-- =============================================================================
-- More component examples (not attached above; try cube:addScript("Collectible"))
-- =============================================================================

-- CharacterController: physics-based movement with a ground raycast.
CharacterController = {}
CharacterController.__index = CharacterController

function CharacterController:new()
    local self = setmetatable({}, CharacterController)
    self.speed = 5.0
    self.jumpForce = 8.0
    self.gravity = -20.0
    self.velocityY = 0
    self.isGrounded = false
    self.groundCheckDistance = 0.1
    return self
end

function CharacterController:init()
    print("[CharacterController] Initialized")
end

function CharacterController:update(dt)
    local go = self.gameObject
    local pos = go.position

    -- Ground detection via raycast
    local rayStart = vec3.new(pos.x, pos.y + 0.5, pos.z)
    local rayEnd = vec3.new(pos.x, pos.y - self.groundCheckDistance, pos.z)
    local hit = atmos.physics.raycast(rayStart, rayEnd)

    if hit then
        self.isGrounded = true
        self.velocityY = 0
        -- Snap to ground
        pos.y = hit.point.y
    else
        self.isGrounded = false
    end

    -- Horizontal movement
    local move = vec3.new(0, 0, 0)
    if atmos.input.isKeyDown(atmos.keys.W) then move.z = 1 end
    if atmos.input.isKeyDown(atmos.keys.S) then move.z = -1 end
    if atmos.input.isKeyDown(atmos.keys.A) then move.x = -1 end
    if atmos.input.isKeyDown(atmos.keys.D) then move.x = 1 end

    -- Normalize horizontal movement
    local len = math.sqrt(move.x * move.x + move.z * move.z)
    if len > 0 then
        move.x = move.x / len * self.speed
        move.z = move.z / len * self.speed
    end

    -- Jump
    if self.isGrounded and atmos.input.isKeyPressed(atmos.keys.SPACE) then
        self.velocityY = self.jumpForce
        print("[CharacterController] Jump!")
    end

    -- Apply gravity
    if not self.isGrounded then
        self.velocityY = self.velocityY + self.gravity * dt
    end

    -- Apply movement
    pos.x = pos.x + move.x * dt
    pos.y = pos.y + self.velocityY * dt
    pos.z = pos.z + move.z * dt

    go.position = pos
end

function CharacterController:onCollision(other)
    print("[CharacterController] Collided with: " .. tostring(other))
end

-- Collectible: reacts to collisions (needs rigidbodies on both objects).
Collectible = {}
Collectible.__index = Collectible

function Collectible:new()
    local self = setmetatable({}, Collectible)
    self.collected = false
    return self
end

function Collectible:init()
    print("[Collectible] Spawned")
end

function Collectible:onCollision(other)
    if not self.collected and other.name == "Ball" then
        self.collected = true
        print("[Collectible] Picked up by the ball!")
        -- Disable the object
        self.gameObject.isActive = false
    end
end

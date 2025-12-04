-- AtmosLua Example Game
-- This is a simple example demonstrating the Lua scripting API

local player
local cube
local time = 0

-- =============================================================================
-- Example ScriptableComponent: Spinner
-- A component that rotates a GameObject around an axis
-- =============================================================================
Spinner = {}
Spinner.__index = Spinner

function Spinner:new()
    local self = setmetatable({}, Spinner)
    self.speed = 1.0
    self.axis = vec3(0, 1, 0)  -- Rotate around Y axis by default
    return self
end

function Spinner:init()
    print("[Spinner] Initialized on " .. tostring(self.gameObject))
end

function Spinner:update(dt)
    local go = self.gameObject
    local rot = go.rotation
    rot.y = rot.y + self.speed * dt
    go.rotation = rot
end

-- =============================================================================
-- Example ScriptableComponent: Bobber
-- A component that bobs a GameObject up and down
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
    -- Store the initial Y position
    self.baseY = self.gameObject.position.y
    print("[Bobber] Initialized with baseY=" .. self.baseY)
end

function Bobber:update(dt)
    self.time = self.time + dt
    local pos = self.gameObject.position
    pos.y = self.baseY + math.sin(self.time * self.frequency * math.pi * 2) * self.amplitude
    self.gameObject.position = pos
end

-- =============================================================================
-- Example ScriptableComponent: CharacterController
-- A physics-based character with ground detection and collision handling
-- =============================================================================
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
    local rayStart = vec3(pos.x, pos.y + 0.5, pos.z)
    local rayEnd = vec3(pos.x, pos.y - self.groundCheckDistance, pos.z)
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
    local move = vec3(0, 0, 0)
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

-- =============================================================================
-- Example ScriptableComponent: Collectible
-- Shows collision callback usage
-- =============================================================================
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
    if not self.collected and other.name == "Player" then
        self.collected = true
        print("[Collectible] Picked up by player!")
        -- Disable the object
        self.gameObject.isActive = false
    end
end

-- =============================================================================
-- Game Code
-- =============================================================================

function load()
    print("[Lua] Game loading...")

    -- Create a simple cube with ScriptableComponents
    cube = atmos.world.spawn(0, 0, 0)
    cube.name = "Cube"

    -- Create the cube mesh
    local cubeMesh = atmos.assets.createCubeMesh("TestCube", 1.0)
    cube:addMesh("TestCube")

    -- Add Spinner and Bobber components - cube will rotate and bob automatically!
    cube:addScript("Spinner")
    cube:addScript("Bobber")

    -- Configure the Spinner via its instance table
    local spinner = cube:getScript()
    if spinner then
        spinner.instance.speed = 2.0  -- Faster rotation
    end

    -- Create player entity
    player = atmos.world.spawn(0, 1, -5)
    player.name = "Player"

    print("[Lua] Game loaded!")
    print("[Lua] Press WASD to move, ESC to quit")
    print("[Lua] The cube will rotate and bob using ScriptableComponents!")
end

function update(dt)
    time = time + dt

    -- Note: Cube rotation and bobbing is now handled by ScriptableComponents!
    -- The Spinner and Bobber scripts update automatically via their update() methods.

    -- Simple player movement
    if player then
        local speed = 5.0
        local pos = player.position
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
            player.position = pos
        end
    end

    -- Quit on ESC
    if atmos.input.isKeyDown(atmos.keys.ESCAPE) then
        print("[Lua] Quitting...")
        atmos.quit()
    end

    -- Reload shaders on R
    if atmos.input.isKeyPressed(atmos.keys.R) then
        print("[Lua] Reloading shaders...")
        atmos.graphics.reloadShaders()
    end
end

function draw()
    -- Optional: Custom draw logic
    -- For now, rendering is handled by the C++ engine
end

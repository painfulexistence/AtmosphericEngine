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

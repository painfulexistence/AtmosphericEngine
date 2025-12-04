-- AtmosLua Example Game
-- This is a simple example demonstrating the Lua scripting API

local player
local cube
local time = 0

function load()
    print("[Lua] Game loading...")

    -- Create a simple cube
    cube = atmos.world.spawn(0, 0, 0)
    cube.name = "Cube"

    -- Create the cube mesh
    local cubeMesh = atmos.assets.createCubeMesh("TestCube", 1.0)
    cube:addMesh("TestCube")

    -- Create player entity
    player = atmos.world.spawn(0, 1, -5)
    player.name = "Player"

    print("[Lua] Game loaded!")
    print("[Lua] Press WASD to move, ESC to quit")
end

function update(dt)
    time = time + dt

    -- Rotate the cube
    if cube then
        cube.rotation = vec3(0, time * 0.5, time)
        cube.position = vec3(0, math.sin(time) * 0.5, 0)
    end

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

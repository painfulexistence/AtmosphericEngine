function init()
    camera = {

    }
    lights = {
        
    }
    maze = {
        size = 50,
        roofed = false,
        tiles_to_remove = 2200,
        tile_size = 4.0,
        chism_probability = 10.0,
        win_coord = {0, 0, 0}
    }
    game_state = {
        is_light_flashing = false,
        light_color = {1.0, 1.0, 1.0},
    }
    print('[Script] Game state initialized.')
end

function load()
    print("[script] Scene & world initialized.")
end

function update(dt, time)
    --skybox glm::rotate(glm::mat4(1.0f), time * glm::radians(10.0f), glm::vec3(0, 1, 0))
end

function draw(dt)
    if framework.check_errors then
        check_errors()
    end
    swap_buffers()
end

function on_game_over()
    --print("[script] Gameplay: game over")
    if framework.auto_close then 
        close_window() 
    end
end

function on_complete()
    --print("[script] Gameplay: game completed")
    if framework.auto_close then 
        close_window() 
    end
end


function init()
    init_game_state = {
        is_light_flashing = false,
        maze_size = 30,
        maze_roofed = false,
        tiles_to_remove = 500,
        tile_size = 3.0,
        chism_probability = 10.0
    }
    cameras = {
        {
            --Main camera
            field_of_view = 3.14 / 3.0,
            near_clip_plane = 0.1,
            far_clip_plane = 2000.0,
            eye_offset = {
                x = 0, 
                y = 2, 
                z = 0
            }
        }
    }
    lights = {
        --Main light
        {
            type = 0,
            position = {0, 0, 0}, --stub
            direction = {-0.168, -0.576, -0.8},
            ambient = {0.2, 0.2, 0.2},
            diffuse = {1, 1, 1},
            specular = {1, 1, 1},
            intensity = 1.0,
            attenuation = {0, 0, 0},
            castShadow = 1
        },
        --Auxiliary lights 
        {
            type = 1,
            position = {0, 20, 0},
            direction = {0, 0, 0},  --stub
            ambient = {0.2, 0.2, 0.2},
            diffuse = {1, 1, 1},
            specular = {1, 1, 1},
            intensity = 3.0,
            attenuation = {1, 0.045, 0.0075}
        },
        {
            type = 1,
            position = {-math.random(50), 20, -math.random(50)},
            direction = {0, 0, 0},  --stub
            ambient = {0.2, 0.2, 0.2},
            diffuse = {1, 1, 0},
            specular = {1, 1, 1},
            intensity = 10.0,
            attenuation = {1, 0.045, 0.0075}
        },
        {
            type = 1,
            position = {math.random(50), 20, math.random(50)},
            direction = {0, 0, 0},  --stub
            ambient = {0.2, 0.2, 0.2},
            diffuse = {1, 0, 1},
            specular = {1, 1, 1},
            intensity = 10.0,
            attenuation = {1, 0.045, 0.0075}
        },
        {
            type = 1,
            position = {math.random(50), 20, -math.random(50)},
            direction = {0, 0, 0},  --stub
            ambient = {0.2, 0.2, 0.2},
            diffuse = {0, 1, 1},
            specular = {1, 1, 1},
            intensity = 10.0,
            attenuation = {1, 0.045, 0.0075}
        }
    }
    materials = {
        {
            name = "Metal",
            textureIdx = 4,
            ambient = {.25, .20725, .20725},
            diffuse = {1, .829, .829},
            specular = {.296648, .296648, .296648},
            shininess = 0.088,
            albedo = {0.8, 0.6, 0.4},
            metallic = 1.0,
            roughness = 0.0,
            ao = 1.0
        },
        {
            name = "Sky",
            textureIdx = 1,
            ambient = {.1, .18725, .1745},
            diffuse = {0, 0, 0},
            specular = {0, 0, 0},
            shininess = 0.04,
            albedo = {0.9, 0.9, 0.9},
            metallic = 0.0,
            roughness = 0.1,
            ao = 0.8
        },
        {
            name = "Plaster",
            textureIdx = 0,
            ambient = {.0, .0, .0},
            diffuse = {.5, .0, .0},
            specular = {.7, .6, .6},
            shininess = 0.25,
            albedo = {0.50754, 0.50754, 0.50754},
            metallic = 0.1,
            roughness = 0.1,
            ao = 0.2
        },
        {
            name = "Brick",
            textureIdx = 3,
            ambient = {0.19225, 0.19225, 0.19225},
            diffuse = {0.50754, 0.50754, 0.50754},
            specular = {0.508273, 0.508273, 0.508273},
            shininess = 0.4,
            albedo = {0.4, 0.3, 0.3},
            metallic = 0.0,
            roughness = 0.4,
            ao = 0.5
        }
    }
    textures = {
        {
            path =  "./assets/textures/plaster.jpg"
        },
        {
            path = "./assets/textures/snow.jpg"
        },
        {
            path = "./assets/textures/grassy.jpg"
        },
        {
            path = "./assets/textures/brick.jpg"
        },
        {
            path = "./assets/textures/metal.jpg"
        }
    }
    shaders = {
        color = {
            vert = "./assets/shaders/pbr.vert",
            frag = "./assets/shaders/pbr.frag"
        },
        depth = {
            vert = "./assets/shaders/depth_simple.vert",
            frag = "./assets/shaders/depth_simple.frag"
        },
        depth_cubemap = {
            vert = "./assets/shaders/depth_cubemap.vert",
            frag = "./assets/shaders/depth_cubemap.frag"
        },
        hdr = {
            vert = "./assets/shaders/hdr.vert",
            frag = "./assets/shaders/hdr.frag"
        }
    }
end

function update(dt, time)
    --skybox glm::rotate(glm::mat4(1.0f), time * glm::radians(10.0f), glm::vec3(0, 1, 0))
end

function draw(dt)
    --if check_rendering_errors then
    --    check_errors()
    --end
    --swap_buffers()
end

function on_game_over()
    --print("[script] Gameplay: game over")
    if auto_close_window then 
        close_window() 
    end
end

function on_complete()
    --print("[script] Gameplay: game completed")
    if auto_close_window then 
        close_window() 
    end
end
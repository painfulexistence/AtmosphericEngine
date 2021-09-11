function init()
    game_state = {
        is_light_flashing = false,
        light_color = {1.0, 1.0, 1.0},
    }
    print('[Script] Game state initialized.')

    clear_color = {}
    ambient_color = {0.2, 0.2, 0.2}
    cameras = {
        --Main camera
        {
            fov = 3.14 / 3.0,
            nearZ = 0.1,
            farZ = 2000.0,
            eyeOffset = {0, 2, 0},
            applyGravity = true,
            checkCollisions = true
        }
    }
    lights = {
        --Main light
        {
            type = 0,
            position = {0, 0, 0}, --stub
            direction = {-0.168, -0.576, -0.8},
            ambient = {0.2, 0.2, 0.2},
            diffuse = game_state.light_color,
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
            path =  "./resources/textures/plaster.jpg"
        },
        {
            path = "./resources/textures/snow.jpg"
        },
        {
            path = "./resources/textures/grassy.jpg"
        },
        {
            path = "./resources/textures/brick.jpg"
        },
        {
            path = "./resources/textures/metal.jpg"
        }
    }
    shaders = {
        color = {
            vert = "./resources/shaders/pbr.vert",
            frag = "./resources/shaders/pbr.frag"
        },
        depth = {
            vert = "./resources/shaders/depth_simple.vert",
            frag = "./resources/shaders/depth_simple.frag"
        },
        depth_cubemap = {
            vert = "./resources/shaders/depth_cubemap.vert",
            frag = "./resources/shaders/depth_cubemap.frag"
        },
        hdr = {
            vert = "./resources/shaders/hdr.vert",
            frag = "./resources/shaders/hdr.frag"
        }
    }
    geometries = {
        {
            name = "Cube",
            vertexData = {
                --One geometry can contains many parts
                {

                },
                {

                }
            }
        }
    }
    meshes = {
        {
            name = "maze",
            materialIdx = 0,
            geometryIdx = 0
        }
    }
    maze = {
        size = 50,
        roofed = false,
        tiles_to_remove = 2200,
        tile_size = 4.0,
        chism_probability = 10.0,
        win_coord = {0, 0, 0}
    }
end

function update(dt, time)
    --skybox glm::rotate(glm::mat4(1.0f), time * glm::radians(10.0f), glm::vec3(0, 1, 0))
end

function draw(dt)
    if config.check_rendering_errors then
        check_errors()
    end
    swap_buffers()
end

function on_game_over()
    --print("[script] Gameplay: game over")
    if config.auto_close then 
        close_window() 
    end
end

function on_complete()
    --print("[script] Gameplay: game completed")
    if config.auto_close then 
        close_window() 
    end
end
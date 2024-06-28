
scene = {
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
    },
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
    },
    -- Phong material reference: http://devernay.free.fr/cours/opengl/materials.html
    -- PBR material reference: https://docs.unity3d.com/Manual/StandardShaderMaterialCharts.html
    materials = {
        {
            name = "Metal",
            baseMapId = 7,
            diffuse = {1., 1., 1.},
            specular = {.296648, .296648, .296648},
            ambient = {.25, .20725, .20725},
            shininess = 0.088
        },
        {
            name = "Mud",
            baseMapId = 16,
            normalMapId = 17,
            aoMapId = 18,
            roughnessMapId = 19,
            diffuse = {1., 1., 1.},
            specular = {0, 0, 0},
            ambient = {.1, .18725, .1745},
            shininess = 0.04
        },
        {
            name = "Plaster",
            baseMapId = 5,
            diffuse = {1., 1., 1.},
            specular = {.7, .6, .6},
            ambient = {.0, .0, .0},
            shininess = 0.25
        },
        {
            name = "Brick",
            baseMapId = 12,
            normalMapId = 13,
            aoMapId = 14,
            roughnessMapId = 15,
            diffuse = {0.50754, 0.50754, 0.50754},
            specular = {0.508273, 0.508273, 0.508273},
            ambient = {0.19225, 0.19225, 0.19225},
            shininess = 0.4
        },
        {
            name = "Stones",
            baseMapId = 8,
            normalMapId = 9,
            aoMapId = 10,
            roughnessMapId = 11,
            diffuse = {1., 1., 1.},
            specular = {.7, .6, .6},
            ambient = {.0, .0, .0},
            shininess = 0.25
        },
        {
            name = "Gold",
            baseMapId = 20,
            roughnessMapId = 21,
            metallicMapId = 22,
            diffuse = {1., 1., 1.},
            specular = {.7, .6, .6},
            ambient = {.0, .0, .0},
            shininess = 0.25
        }
    },
    textures = {
        -- Default textures
        {
            path =  "./assets/textures/default_diff.jpg"
        },
        {
            path = "./assets/textures/default_norm.jpg"
        },
        {
            path =  "./assets/textures/default_ao.jpg"
        },
        {
            path = "./assets/textures/default_rough.jpg"
        },
        {
            path =  "./assets/textures/default_metallic.jpg"
        },
        -- Custom textures
        {
            path =  "./assets/textures/rough_plaster.jpg"
        },
        {
            path = "./assets/textures/brick.jpg"
        },
        {
            path = "./assets/textures/rusty_metal.jpg"
        },
        {
            path = "./assets/textures/paving_stones_diff.jpg"
        },
        {
            path = "./assets/textures/paving_stones_norm_gl.jpg"
        },
        {
            path = "./assets/textures/paving_stones_ao.jpg"
        },
        {
            path = "./assets/textures/paving_stones_rough.jpg"
        },
        {
            path = "./assets/textures/medieval_blocks_diff.jpg"
        },
        {
            path = "./assets/textures/medieval_blocks_norm_gl.jpg"
        },
        {
            path = "./assets/textures/medieval_blocks_ao.jpg"
        },
        {
            path = "./assets/textures/medieval_blocks_rough.jpg"
        },
        {
            path = "./assets/textures/brown_mud_leaves_diff.jpg"
        },
        {
            path = "./assets/textures/brown_mud_leaves_norm_gl.jpg"
        },
        {
            path = "./assets/textures/brown_mud_leaves_ao.jpg"
        },
        {
            path = "./assets/textures/brown_mud_leaves_rough.jpg"
        },
        {
            path = "./assets/textures/gold_diff.jpg"
        },
        {
            path = "./assets/textures/gold_rough.jpg"
        },
        {
            path = "./assets/textures/gold_metallic.jpg"
        }
    },
    shaders = {
        color = {
            vert = "./assets/shaders/simple.vert",
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
            frag = "./assets/shaders/hdr_ca.frag"
        }
    }
}

function init()
    init_game_state = {
        is_light_flashing = true,
        maze_size = 30,
        maze_roofed = false,
        tiles_to_remove = 500,
        tile_size = 5.0,
        chism_probability = 10.0
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
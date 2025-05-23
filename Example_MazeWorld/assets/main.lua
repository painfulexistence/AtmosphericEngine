
scenes = {
    {
        entities = {
            player = {
                components = {
                    camera = {
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
            },
            main_light = {
                position = {0, 0, 0},
                components = {
                    light = {
                        type = 0,
                        direction = {-0.168, -0.576, -0.8},
                        ambient = {0.2, 0.2, 0.2},
                        diffuse = {1, 1, 1},
                        specular = {1, 1, 1},
                        intensity = 1.0,
                        castShadow = 1
                    }
                }
            },
            aux_light_1 = {
                position = {0, 10, 0},
                components = {
                    light = {
                        type = 1,
                        ambient = {0.2, 0.2, 0.2},
                        diffuse = {1, 1, 1},
                        specular = {1, 1, 1},
                        intensity = 3.0,
                        attenuation = {1, 0.045, 0.0075},
                        castShadow = 1
                    }
                }
            },
            aux_light_2 = {
                position = {-math.random(50), 10, -math.random(50)},
                components = {
                    light = {
                        type = 1,
                        ambient = {0.2, 0.2, 0.2},
                        diffuse = {1, 1, 0},
                        specular = {1, 1, 1},
                        intensity = 10.0,
                        attenuation = {1, 0.045, 0.0075},
                        castShadow = 1
                    }
                }
            },
            aux_light_3 = {
                position = {math.random(50), 10, math.random(50)},
                components = {
                    light = {
                        type = 1,
                        ambient = {0.2, 0.2, 0.2},
                        diffuse = {1, 0, 1},
                        specular = {1, 1, 1},
                        intensity = 10.0,
                        attenuation = {1, 0.045, 0.0075},
                        castShadow = 1
                    }
                }
            },
            aux_light_4 = {
                position = {math.random(50), 10, -math.random(50)},
                components = {
                    light = {
                        type = 1,
                        ambient = {0.2, 0.2, 0.2},
                        diffuse = {0, 1, 1},
                        specular = {1, 1, 1},
                        intensity = 10.0,
                        attenuation = {1, 0.045, 0.0075},
                        castShadow = 1
                    }
                }
            }
        },
        prefabs = {
            {
                name = "Player",
                shape = {
                    type = "capsule",
                    radius = 0.5,
                    height = 3.0
                },
            },
            {
                name = "Skybox",
                mesh = {
                    type = "cube",
                    size = 800.0,
                    material_id = 1
                },
            },
            {
                name = "Ball",
                mesh = {
                    type = "sphere",
                    radius = 0.5,
                    material_id = 0
                },
                shape = {
                    type = "sphere",
                    radius = 0.5
                }
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
            },
            {
                name = "Terrain",
                heightMapId = 23,
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
            },
            -- Terrain maps
            {
                path = "./assets/textures/heightmap.jpg"
            },
            {
                path = "./assets/textures/aim.png"
            }
        },
        shaders = {
            color = {
                vert = "./assets/shaders/tbn.vert",
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
            },
            terrain = {
                vert = "./assets/shaders/terrain.vert",
                frag = "./assets/shaders/terrain.frag",
                tesc = "./assets/shaders/terrain.tesc",
                tese = "./assets/shaders/terrain.tese"
            },
            terrain_debug = {
                vert = "./assets/shaders/simple.vert",
                frag = "./assets/shaders/flat.frag",
            },
            debug_line = {
                vert = "./assets/shaders/debug.vert",
                frag = "./assets/shaders/flat.frag",
            },
            canvas = {
                vert = "./assets/shaders/canvas.vert",
                frag = "./assets/shaders/canvas.frag",
            }
        }
    }
}

function init()
    init_game_state = {
        is_light_flashing = false,
        maze_size = 30,
        maze_roofed = false,
        tiles_to_remove = 500,
        tile_size = 3.0,
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
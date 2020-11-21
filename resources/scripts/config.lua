framework = {
    auto_close = false,
    check_errors = true
}

shaders = {
    color = {
        vert = "./resources/shaders/multilight_shadow.vert",
        frag = "./resources/shaders/multilight_shadow.frag"
    },
    depth = {
        vert = "./resources/shaders/depth_simple.vert",
        frag = "./resources/shaders/depth_simple.frag"
    }
}

print('[Script] Configured.')
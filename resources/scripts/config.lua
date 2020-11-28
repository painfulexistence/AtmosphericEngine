framework = {
    auto_close = false,
    check_errors = true
}

textures = {

}

aux_shadow_count = 1

shaders = {
    color = {
        vert = "./resources/shaders/multilight_shadow.vert",
        frag = "./resources/shaders/multilight_shadow.frag"
    },
    depth = {
        texture = {
            vert = "./resources/shaders/depth_simple.vert",
            frag = "./resources/shaders/depth_simple.frag"
        },
        cubemap = {
            vert = "./resources/shaders/depth_cubemap.vert",
            frag = "./resources/shaders/depth_cubemap.frag"
        }
    },
    hdr = {
        vert = "./resources/shaders/hdr.vert",
        frag = "./resources/shaders/hdr.frag"
    }
}

print('[Script] Configured.')
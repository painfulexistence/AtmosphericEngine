set(TARGET_NAME AtmosLua)

find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)
# lua_static and sol2::sol2 are PUBLIC deps of AtmosphericEngine — no find_package needed

# Source files
set(LUA_FRONTEND_SOURCES
    main.cpp
    lua_application.cpp
    scriptable_component.cpp
    bindings/bind_core.cpp
    bindings/bind_input.cpp
    bindings/bind_world.cpp
    bindings/bind_graphics.cpp
    bindings/bind_physics.cpp
)
if(NOT EMSCRIPTEN)
    list(APPEND LUA_FRONTEND_SOURCES bindings/bind_audio.cpp)
endif()

add_executable(${TARGET_NAME} ${LUA_FRONTEND_SOURCES})

set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}
)

target_include_directories(${TARGET_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/AtmosphericEngine/include
        ${CMAKE_SOURCE_DIR}/AtmosphericEngine/external/stb
        # LUA_INCLUDE_DIR removed: Lua headers propagate from AtmosphericEngine → lua_static
)

# Base link libraries (all platforms)
target_link_libraries(${TARGET_NAME}
    PRIVATE
        AtmosphericEngine   # pulls in lua_static + sol2::sol2 transitively
        glm::glm
        fmt::fmt
        BulletSoftBody
        BulletDynamics
        BulletCollision
        LinearMath
)

if(EMSCRIPTEN)
    # ── Emscripten / WebAssembly build ──────────────────────────────────────
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    # Stage shaders BEFORE the link step (--preload-file requires the source
    # path to exist at link time; POST_BUILD is too late).
    # Textures and scripts are loaded dynamically via FileSystem::Prefetch.
    set(EM_PRELOAD_STAGING_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/assets)

    add_custom_target(stage_${TARGET_NAME}_em_preload
        COMMAND ${CMAKE_COMMAND} -E make_directory ${EM_PRELOAD_STAGING_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/AtmosphericEngine/default_assets/shaders
            ${EM_PRELOAD_STAGING_DIR}/shaders
        COMMENT "Staging Emscripten preload assets (shaders) for ${TARGET_NAME}"
    )
    add_dependencies(${TARGET_NAME} stage_${TARGET_NAME}_em_preload)

    # -sFETCH=1               : emscripten_fetch API (used by FileSystem on web)
    # -sFORCE_FILESYSTEM=1    : ensure FS.writeFile / FS.mkdir are linked in
    #                           even when Emscripten's dead-code elimination
    #                           might otherwise strip them (needed by FileSystem
    #                           MEMFS writes for .lua / .json / .glsl files)
    # -sALLOW_MEMORY_GROWTH=1 : let the WASM heap grow; prevents OOM when
    #                           multiple KTX2 textures are transcoded in sequence
    target_link_options(${TARGET_NAME} PRIVATE
        -sWASM=1 -O3 -sUSE_GLFW=3
        -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1
        -sFETCH=1 -sFORCE_FILESYSTEM=1 -sALLOW_MEMORY_GROWTH=1
        -sASSERTIONS=1
        --preload-file ${EM_PRELOAD_STAGING_DIR}/shaders@assets/shaders
    )

else()
    # ── Native build ─────────────────────────────────────────────────────────
    find_package(spdlog CONFIG REQUIRED)
    find_package(glad CONFIG REQUIRED)

    target_link_libraries(${TARGET_NAME}
        PRIVATE
            glad::glad
            spdlog::spdlog
    )
    target_link_libraries(${TARGET_NAME} PRIVATE raudio SDL3::SDL3)
endif()

# ── Post-build: copy all default assets (shaders, textures, fonts, ...) ─────
# For native builds, copies everything into place.
# For web builds, copies KTX2 textures so the dev-server can serve them;
# scripts are copied separately (below) for the same reason.
add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/AtmosphericEngine/default_assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/assets
    COMMENT "Copying default assets for ${TARGET_NAME}"
)

# Create scripts output directory and copy example scripts
add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/assets/scripts
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    add_custom_target(copy_assets_for_${TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/assets
    )
    add_dependencies(${TARGET_NAME} copy_assets_for_${TARGET_NAME})
endif()

set(GAME_NAME HelloWorld)
set(GAME_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)

find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)
# lua_static is a PUBLIC dep of AtmosphericEngine — no find_package needed

add_executable(${GAME_NAME}
    main.cpp
)
set_target_properties(${GAME_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}
)

target_include_directories(${GAME_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/AtmosphericEngine/include)

# Base dependencies (available on all platforms)
target_link_libraries(${GAME_NAME} PRIVATE
    AtmosphericEngine   # pulls in lua_static transitively
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

    # ── Stage shader and font assets BEFORE the link step ───────────────────
    # emscripten --preload-file requires the source path to exist at link time.
    # POST_BUILD is too late; we use a PRE_LINK target instead.
    #
    # Textures are intentionally NOT included here — they are loaded at runtime
    # via FileSystem::Prefetch (emscripten_fetch + IndexedDB), which avoids
    # bloating the .data bundle and reduces peak WASM heap usage.
    set(EM_PRELOAD_STAGING_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}/assets)

    add_custom_target(stage_${GAME_NAME}_em_preload
        COMMAND ${CMAKE_COMMAND} -E make_directory ${EM_PRELOAD_STAGING_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/AtmosphericEngine/default_assets/shaders
            ${EM_PRELOAD_STAGING_DIR}/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts
            ${EM_PRELOAD_STAGING_DIR}/fonts
        COMMENT "Staging Emscripten preload assets (shaders, fonts) for ${GAME_NAME}"
    )
    add_dependencies(${GAME_NAME} stage_${GAME_NAME}_em_preload)

    # ── Emscripten link options ──────────────────────────────────────────────
    # --preload-file  shaders + fonts only (small, needed synchronously).
    # Textures: fetched asynchronously via FileSystem::Prefetch in main.cpp.
    # -sFETCH=1       : inherited from AtmosphericEngine but repeated here for
    #                   the executable's link step (FileSystem web implementation).
    # -sALLOW_MEMORY_GROWTH=1 : allows WASM heap to grow dynamically; avoids
    #                   OOM when loading multiple large textures in sequence.
    target_link_options(${GAME_NAME} PRIVATE
        -sWASM=1
        -O3
        -sUSE_GLFW=3
        -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1
        -sFETCH=1
        -sALLOW_MEMORY_GROWTH=1
        -sASSERTIONS=1
        --preload-file ${EM_PRELOAD_STAGING_DIR}/shaders@assets/shaders
    )

else()
    # ── Native build ─────────────────────────────────────────────────────────
    find_package(spdlog CONFIG REQUIRED)
    find_package(glad CONFIG REQUIRED)

    target_link_libraries(${GAME_NAME} PRIVATE
        glad::glad
        spdlog::spdlog
        raudio
    )

    target_link_libraries(${GAME_NAME} PRIVATE SDL3::SDL3)
endif()

# ── Post-build: copy all default assets to output dir ───────────────────────
# (For native builds this copies textures/shaders/fonts/etc. into place.
#  For Emscripten, this copies KTX2 textures so the dev-server can serve them.)
add_custom_command(
    TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/AtmosphericEngine/default_assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}/assets
    COMMENT "Copying default assets for ${GAME_NAME}"
)

if(EXISTS ${GAME_ASSETS_DIR})
    add_custom_target(copy_${GAME_NAME}_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${GAME_ASSETS_DIR}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}/assets
    )
    add_dependencies(${GAME_NAME} copy_${GAME_NAME}_assets)
endif()

set(GAME_NAME Maze)
set(GAME_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)

find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)

add_executable(${GAME_NAME}
    main.cpp
    components/character_controller_component.cpp
    components/weapon_component.cpp
    components/hud_component.cpp
)
set_target_properties(${GAME_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}
)

target_include_directories(${GAME_NAME} PUBLIC 
    ${CMAKE_SOURCE_DIR}/AtmosphericEngine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/components
)

# Base dependencies (available on all platforms)
target_link_libraries(${GAME_NAME} PRIVATE
    AtmosphericEngine
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

    # ── Stage shader, font, ui, sounds and script assets BEFORE the link step ──
    # emscripten --preload-file requires the source path to exist at link time.
    set(EM_PRELOAD_STAGING_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}/assets)

    add_custom_target(stage_${GAME_NAME}_em_preload
        COMMAND ${CMAKE_COMMAND} -E make_directory ${EM_PRELOAD_STAGING_DIR}
        # Copy engine default shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/AtmosphericEngine/default_assets/shaders
            ${EM_PRELOAD_STAGING_DIR}/shaders
        # Copy Maze game fonts, ui, sounds, and Lua scripts
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts
            ${EM_PRELOAD_STAGING_DIR}/fonts
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/ui
            ${EM_PRELOAD_STAGING_DIR}/ui
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/sounds
            ${EM_PRELOAD_STAGING_DIR}/sounds
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/config.lua
            ${EM_PRELOAD_STAGING_DIR}/config.lua
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/main.lua
            ${EM_PRELOAD_STAGING_DIR}/main.lua
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/manifest.lua
            ${EM_PRELOAD_STAGING_DIR}/manifest.lua
        COMMENT "Staging Emscripten preload assets (shaders, fonts, UI, sounds, scripts) for ${GAME_NAME}"
    )
    add_dependencies(${GAME_NAME} stage_${GAME_NAME}_em_preload)

    # ── Emscripten link options ──────────────────────────────────────────────
    # Using "SHELL:" prefix prevents CMake from automatically deduplicating multiple 
    # "--preload-file" flags, which would cause the subsequent paths to be treated as raw inputs.
    set(MAZE_EM_LINK_OPTIONS
        -sALLOW_MEMORY_GROWTH=1
        -sDISABLE_EXCEPTION_CATCHING=0
        -sEXPORTED_FUNCTIONS=["_main","_printWasmMemoryStats"]
        "SHELL:--preload-file ${EM_PRELOAD_STAGING_DIR}/shaders@assets/shaders"
        "SHELL:--preload-file ${EM_PRELOAD_STAGING_DIR}/fonts@assets/fonts"
        "SHELL:--preload-file ${EM_PRELOAD_STAGING_DIR}/ui@assets/ui"
        "SHELL:--preload-file ${EM_PRELOAD_STAGING_DIR}/sounds@assets/sounds"
        "SHELL:--preload-file ${EM_PRELOAD_STAGING_DIR}/config.lua@assets/config.lua"
        "SHELL:--preload-file ${EM_PRELOAD_STAGING_DIR}/main.lua@assets/main.lua"
        "SHELL:--preload-file ${EM_PRELOAD_STAGING_DIR}/manifest.lua@assets/manifest.lua"
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND MAZE_EM_LINK_OPTIONS
            -g
            -sASSERTIONS=2
        )
    else()
        list(APPEND MAZE_EM_LINK_OPTIONS
            -O3
            -sASSERTIONS=1
        )
    endif()

    target_link_options(${GAME_NAME} PRIVATE ${MAZE_EM_LINK_OPTIONS})

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

    # Convert textures to KTX2 under WebAssembly
    ae_auto_convert_textures(${GAME_NAME}
        ${GAME_ASSETS_DIR}/textures
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}/assets/textures
    )
endif()
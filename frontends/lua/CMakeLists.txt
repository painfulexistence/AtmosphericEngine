set(TARGET_NAME AtmosLua)

# Source files
set(LUA_FRONTEND_SOURCES
    main.cpp
    lua_application.cpp
    scriptable_component.cpp
    bindings/bind_core.cpp
    bindings/bind_input.cpp
    bindings/bind_world.cpp
    bindings/bind_graphics.cpp
)

# Create executable
add_executable(${TARGET_NAME} ${LUA_FRONTEND_SOURCES})

# Set output directory
set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}
)

# Include directories
target_include_directories(${TARGET_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/AtmosphericEngine/include
        ${CMAKE_SOURCE_DIR}/AtmosphericEngine/external/sol2/include
        ${LUA_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(${TARGET_NAME}
    PRIVATE
        AtmosphericEngine
        glm::glm
        fmt::fmt
        EnTT::EnTT
        BulletSoftBody
        BulletDynamics
        BulletCollision
        LinearMath
        ${LUA_LIBRARIES}
        sol2
        glad::glad
        spdlog::spdlog
)

# Platform-specific linking
if(AE_USE_AUDIO)
    target_link_libraries(${TARGET_NAME} PRIVATE raudio)
endif()

if(AE_USE_SDL3)
    target_link_libraries(${TARGET_NAME} PRIVATE SDL3::SDL3)
elseif(AE_USE_SDL2)
    target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2)
else()
    target_link_libraries(${TARGET_NAME} PRIVATE glfw)
endif()

# Copy default assets
add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/AtmosphericEngine/default_assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/assets
)

# Create scripts directory with example main.lua
add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/assets/scripts
)

# Copy example main.lua if it exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/example_main.lua)
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/example_main.lua
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/assets/scripts/main.lua
    )
endif()

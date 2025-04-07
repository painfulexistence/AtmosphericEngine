if(NOT USE_EMSCRIPTEN)
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
    find_package(glad CONFIG REQUIRED)
endif()
find_package(glm CONFIG REQUIRED)

option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_BULLET_ROBOTICS_EXTRA OFF)
option(BUILD_BULLET_ROBOTICS_GUI_EXTRA OFF)
option(BUILD_UNIT_TESTS OFF)
option(BUILD_LUA_AS_DLL OFF)
option(BUILD_SHARED_LIBS OFF)
find_package(Bullet CONFIG REQUIRED)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(lua REQUIRED)

add_subdirectory(external/entt)
add_subdirectory(external/sol2)
add_subdirectory(external/raudio/projects/CMake)

#file(GLOB SOURCES *.cpp RECURSIVE EXCLUDE external)
set(SOURCES
    application.cpp
    window.cpp
    server.cpp
    console.cpp
    game_object.cpp
    impostor.cpp
    light.cpp
    camera.cpp
    mesh.cpp
    frustum.cpp
    renderable.cpp
    drawable_2d.cpp
    shader.cpp
    graphics_server.cpp
    physics_server.cpp
    physics_debug_drawer.cpp
    input.cpp
    entity.cpp
    script.cpp
    scene.cpp
    asset_manager.cpp
    job_system.cpp
    utility/file.cpp
    audio_manager.cpp
)

set(SOURCES_EXT
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
)

add_library(AtmosphericEngine STATIC ${SOURCES} ${SOURCES_EXT})

target_include_directories(AtmosphericEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${BULLET_INCLUDE_DIR} ${LUA_INCLUDE_DIR} external/stb external/imgui)
target_precompile_headers(AtmosphericEngine PRIVATE pch.hpp)
target_link_libraries(AtmosphericEngine
    PUBLIC
        glm::glm fmt::fmt spdlog::spdlog EnTT::EnTT BulletSoftBody BulletDynamics BulletCollision LinearMath ${LUA_LIBRARIES} sol2 raudio
)
if(USE_EMSCRIPTEN)
    target_link_options(AtmosphericEngine PRIVATE -sWASM=1 -O3 -sUSE_GLFW=3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1)
else()
    target_link_libraries(AtmosphericEngine
        PUBLIC
            glad::glad glfw
    )
endif()
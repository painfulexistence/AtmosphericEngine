
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
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
find_package(lua REQUIRED)

add_subdirectory(external/entt)
add_subdirectory(external/sol2)

#file(GLOB SOURCES *.cpp RECURSIVE EXCLUDE external)
set(SOURCES
    application.cpp
    window.cpp
    server.cpp
    console.cpp
    component_factory.cpp
    game_object.cpp
    impostor.cpp
    light.cpp
    camera.cpp
    mesh.cpp
    frustum.cpp
    renderable.cpp
    shader.cpp
    graphics_server.cpp
    physics_server.cpp
    physics_world.cpp
    physics_debug_drawer.cpp
    input.cpp
    entity.cpp
    script.cpp
    utility/file.cpp
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
target_link_libraries(AtmosphericEngine PUBLIC GLEW::GLEW glfw glm::glm fmt::fmt EnTT::EnTT BulletSoftBody BulletDynamics BulletCollision LinearMath ${LUA_LIBRARIES} sol2)
target_precompile_headers(AtmosphericEngine PUBLIC pch.hpp)
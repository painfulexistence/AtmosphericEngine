option(BUILD_SHARED_LIBS OFF)
if(NOT AE_USE_EMSCRIPTEN)
    if(AE_USE_SDL3)
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-3.2.12 # TODO: update to 3.2.14 after upgrading imgui
        )
        set(SDL_SHARED OFF CACHE BOOL "" FORCE)
        set(SDL_STATIC ON CACHE BOOL "" FORCE)
        set(SDL_TESTS OFF CACHE BOOL "" FORCE)
        set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(SDL_TESTS_LIBRARY OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(SDL3)
    elseif(AE_USE_SDL2)
        FetchContent_Declare(
            SDL2
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-2.32.6
        )
        set(SDL_SHARED OFF CACHE BOOL "" FORCE)
        set(SDL_STATIC ON CACHE BOOL "" FORCE)
        set(SDL_TEST OFF CACHE BOOL "" FORCE)
        set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(SDL2)
    else()
        FetchContent_Declare(glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
        )
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glfw)
    endif()
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
find_package(Bullet CONFIG REQUIRED)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(lua REQUIRED)

add_subdirectory(external/entt)
add_subdirectory(external/sol2)
if(AE_USE_AUDIO)
    add_subdirectory(external/raudio/projects/CMake)
endif()

#file(GLOB SOURCES src/*.cpp RECURSIVE)
set(SOURCES
    src/application.cpp
    src/game_layer.cpp
    src/editor_layer.cpp
    src/server.cpp
    src/console.cpp
    src/game_object.cpp
    src/transform_component.cpp
    src/rigidbody_component.cpp
    src/light_component.cpp
    src/camera_component.cpp
    src/mesh.cpp
    src/frustum.cpp
    src/mesh_component.cpp
    src/sprite_component.cpp
    src/terrain_component.cpp
    src/shader.cpp
    src/graphics_server.cpp
    src/renderer.cpp
    src/physics_server.cpp
    src/physics_debug_drawer.cpp
    src/input.cpp
    src/entity.cpp
    src/script.cpp
    src/scene.cpp
    src/asset_manager.cpp
    src/job_system.cpp
    src/bullet_task_scheduler.cpp
    src/file.cpp
)
if(AE_USE_AUDIO)
    list(APPEND SOURCES src/audio_manager.cpp)
endif()
if(AE_USE_SDL3)
    list(APPEND SOURCES src/window_sdl3.cpp)
elseif(AE_USE_SDL2)
    list(APPEND SOURCES src/window_sdl2.cpp)
else()
    list(APPEND SOURCES src/window_glfw.cpp)
endif()

set(SOURCES_EXT
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
if(AE_USE_SDL3)
    list(APPEND SOURCES_EXT external/imgui/backends/imgui_impl_sdl3.cpp)
elseif(AE_USE_SDL2)
    list(APPEND SOURCES_EXT external/imgui/backends/imgui_impl_sdl2.cpp)
else()
    list(APPEND SOURCES_EXT external/imgui/backends/imgui_impl_glfw.cpp)
endif()

add_library(AtmosphericEngine STATIC ${SOURCES} ${SOURCES_EXT})

target_include_directories(AtmosphericEngine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${BULLET_INCLUDE_DIR}
        ${LUA_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Atmospheric
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_precompile_headers(AtmosphericEngine PRIVATE src/pch.hpp)
target_link_libraries(AtmosphericEngine
    PUBLIC
        glm::glm fmt::fmt EnTT::EnTT BulletSoftBody BulletDynamics BulletCollision LinearMath ${LUA_LIBRARIES} sol2 raudio
)
if (AE_USE_AUDIO)
    target_link_libraries(AtmosphericEngine
        PUBLIC
            raudio
    )
endif()
if(AE_USE_EMSCRIPTEN)
    target_compile_definitions(AtmosphericEngine PUBLIC BT_THREADSAFE=1)
    target_compile_options(AtmosphericEngine PRIVATE)
    if(AE_USE_SDL3)
        target_link_options(AtmosphericEngine PRIVATE -sWASM=1 -O3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1)
    elseif(AE_USE_SDL2)
        target_link_options(AtmosphericEngine PRIVATE -sWASM=1 -O3 -sUSE_SDL=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1)
    else()
        target_link_options(AtmosphericEngine PRIVATE -sWASM=1 -O3 -sUSE_GLFW=3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1)
    endif()
else()
    target_compile_definitions(AtmosphericEngine PUBLIC BT_THREADSAFE=1)
    target_link_libraries(AtmosphericEngine
        PUBLIC
            glad::glad
            spdlog::spdlog
    )
    if(AE_USE_SDL3)
        target_link_libraries(AtmosphericEngine
            PUBLIC
                SDL3::SDL3
        )
    elseif(AE_USE_SDL2)
        target_link_libraries(AtmosphericEngine
            PUBLIC
                SDL2::SDL2
        )
    else()
        target_link_libraries(AtmosphericEngine
            PUBLIC
                glfw
        )
    endif()
endif()
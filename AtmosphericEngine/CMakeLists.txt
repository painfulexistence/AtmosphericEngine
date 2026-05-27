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
find_package(Tracy CONFIG)
option(AE_USE_TRACY "Enable Tracy profiler instrumentation" OFF)

option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_BULLET_ROBOTICS_EXTRA OFF)
option(BUILD_BULLET_ROBOTICS_GUI_EXTRA OFF)
option(BUILD_UNIT_TESTS OFF)
option(BUILD_LUA_AS_DLL OFF)
find_package(Bullet CONFIG REQUIRED)

# Box2D for 2D physics
find_package(box2d CONFIG REQUIRED)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(RmlUi REQUIRED)
find_package(flatbuffers CONFIG REQUIRED)

# ── Lua 5.4 (built from source) ───────────────────────────────────────────────
# Building from source rather than find_package(lua) because:
#   1. Emscripten must compile Lua to WASM — system lua.a is host-arch only.
#   2. Avoids architecture mismatch on Apple Silicon (arm64 vs x86_64).
#   3. Version is pinned — no dependency on whatever vcpkg/Homebrew installed.
FetchContent_Declare(lua_src
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG        v5.4.7
    GIT_SHALLOW    TRUE
    UPDATE_DISCONNECTED TRUE
)
FetchContent_GetProperties(lua_src)
if(NOT lua_src_POPULATED)
    FetchContent_Populate(lua_src)
endif()

# Lua's repo has all .c files in the root (no src/ subdir).
# Exclude the standalone interpreter (lua.c) and compiler (luac.c).
file(GLOB LUA_CORE_SOURCES "${lua_src_SOURCE_DIR}/*.c")
list(FILTER LUA_CORE_SOURCES EXCLUDE REGEX ".*(lua|luac)\\.c$")

add_library(lua_static STATIC ${LUA_CORE_SOURCES})
target_include_directories(lua_static PUBLIC "${lua_src_SOURCE_DIR}")
if(UNIX AND NOT EMSCRIPTEN)
    # Enable POSIX APIs (better file I/O, signals, etc.) on Linux / macOS.
    # Not needed on Windows or Emscripten.
    target_compile_definitions(lua_static PRIVATE LUA_USE_POSIX)
endif()
# Suppress warnings from Lua's C code (not our code, not our problem)
if(MSVC)
    target_compile_options(lua_static PRIVATE /W0)
else()
    target_compile_options(lua_static PRIVATE -w)
endif()

# ── sol2 v3.3.0 (header-only) ─────────────────────────────────────────────────
# Previously a git submodule (external/sol2); switched to FetchContent so there
# is no .gitmodules entry and no submodule init step required.
# We use FetchContent_Populate (not MakeAvailable) to avoid running sol2's own
# CMakeLists.txt, which would call find_package(Lua) and conflict with lua_static.
FetchContent_Declare(sol2_src
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG        v3.3.0
    GIT_SHALLOW    TRUE
    UPDATE_DISCONNECTED TRUE
)
FetchContent_GetProperties(sol2_src)
if(NOT sol2_src_POPULATED)
    FetchContent_Populate(sol2_src)
endif()

# sol2 is header-only; create a minimal INTERFACE target manually.
add_library(sol2_iface INTERFACE)
target_include_directories(sol2_iface INTERFACE "${sol2_src_SOURCE_DIR}/include")
# Link lua_static as INTERFACE so that:
#   1. sol2 headers resolve <lua.h> / <lua.hpp> from our Lua 5.4 source,
#      not from any system/vcpkg installation.
#   2. Include order is guaranteed: our lua_static dir comes first.
target_link_libraries(sol2_iface INTERFACE lua_static)
# Alias matches the target name produced by find_package(sol2 CONFIG REQUIRED),
# so existing target_link_libraries(... sol2::sol2) lines continue to work.
add_library(sol2::sol2 ALIAS sol2_iface)

# Generate FlatBuffers headers from schema
set(FLATBUFFERS_SCHEMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/schemas)
set(FLATBUFFERS_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${FLATBUFFERS_GENERATED_DIR})

add_custom_command(
    OUTPUT ${FLATBUFFERS_GENERATED_DIR}/Scene_generated.h
    COMMAND flatbuffers::flatc --cpp -o ${FLATBUFFERS_GENERATED_DIR} ${FLATBUFFERS_SCHEMA_DIR}/Scene.fbs
    DEPENDS ${FLATBUFFERS_SCHEMA_DIR}/Scene.fbs
    COMMENT "Generating FlatBuffers headers for Scene"
)

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
    src/render_mesh.cpp
    src/csg.cpp
    src/frustum.cpp
    src/mesh_component.cpp
    src/sprite_component.cpp
    src/sprite_3d_component.cpp
    src/terrain_component.cpp
    src/shader.cpp
    src/graphics_server.cpp
    src/renderer.cpp
    src/physics_server.cpp
    src/physics_debug_drawer.cpp
    src/input.cpp
    src/scene.cpp
    src/action.cpp
    src/action_manager.cpp
    src/animator_2d.cpp
    src/asset_manager.cpp
    src/font_manager.cpp
    src/job_system.cpp
    src/bullet_task_scheduler.cpp
    src/particle_server.cpp
    src/particle_emitter.cpp
    src/file.cpp
    src/rmlui_renderer.cpp
    src/rmlui_system.cpp
    src/rmlui_manager.cpp
    src/ui_page_manager.cpp
    src/batch_renderer_2d.cpp
    src/physics_server_2d.cpp
    src/rigidbody_2d_component.cpp
    src/render_texture.cpp
    src/mesh_builder.cpp
    src/shape_renderer_component.cpp
    src/scene_loader.cpp
    ${FLATBUFFERS_GENERATED_DIR}/Scene_generated.h
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
# FileSystem: platform-agnostic async I/O with in-process cache.
#   Web    : emscripten_fetch + IndexedDB + MEMFS writes for text assets.
#   Native : synchronous fread + JobSystem-parallel Prefetch.
# Compiled on all platforms (not just Emscripten).
list(APPEND SOURCES src/file_system.cpp)

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

# ============================================================================
# Basis Universal transcoder (KTX2 GPU-compressed texture support)
# Only the transcoder is compiled — the encoder is not needed at runtime.
# Supported target formats for WebGL2:
#   - ETC2  (GLES3 required, best for web/mobile)
#   - S3TC  (desktop browsers, via extension check)
# Developers convert source textures to KTX2 with:
#   basisu -ktx2 -mipmap texture.png
#   toktx --t2 --encode etc1s --mipmap out.ktx2 input.png
# ============================================================================
if(AE_USE_BASIS_UNIVERSAL)
    FetchContent_Declare(
        basis_universal
        GIT_REPOSITORY https://github.com/BinomialLLC/basis_universal.git
        GIT_TAG        900e40fb5d2502927360fe2f31762bdbb624455b  # v1_16_4 (2023-11)
        GIT_SHALLOW    TRUE
        UPDATE_DISCONNECTED TRUE
    )
    FetchContent_GetProperties(basis_universal)
    if(NOT basis_universal_POPULATED)
        # Do NOT process basisu's own CMakeLists.txt (it drags in encoder, etc.)
        FetchContent_Populate(basis_universal)
    endif()

    add_library(basisu_transcoder STATIC
        ${basis_universal_SOURCE_DIR}/transcoder/basisu_transcoder.cpp
    )
    target_include_directories(basisu_transcoder PUBLIC
        ${basis_universal_SOURCE_DIR}
        ${basis_universal_SOURCE_DIR}/transcoder
    )
    # Disable formats not needed for web (reduces transcoder binary size)
    target_compile_definitions(basisu_transcoder PUBLIC
        BASISD_SUPPORT_KTX2=1          # Enable KTX2 container
        BASISD_SUPPORT_KTX2_ZSTD=0     # Zstandard supercompression not needed
        BASISD_SUPPORT_PVRTC1=0        # PowerVR only (legacy mobile)
        BASISD_SUPPORT_PVRTC2=0        # PowerVR2 only (legacy mobile)
        BASISD_SUPPORT_FXT1=0          # Intel FXT1 (obsolete)
        BASISD_SUPPORT_ATC=0           # ATI (obsolete)
    )
    if(MSVC)
        target_compile_options(basisu_transcoder PRIVATE /W0)
    else()
        target_compile_options(basisu_transcoder PRIVATE
            -Wno-unused-variable
            -Wno-unused-function
            -Wno-unused-but-set-variable
        )
    endif()
endif()

add_library(AtmosphericEngine STATIC ${SOURCES} ${SOURCES_EXT})

target_include_directories(AtmosphericEngine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${BULLET_INCLUDE_DIR}
        # lua_static has PUBLIC include dirs → Lua headers propagate automatically
        ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Atmospheric
        ${FLATBUFFERS_GENERATED_DIR}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_precompile_headers(AtmosphericEngine PRIVATE src/pch.hpp)
target_link_libraries(AtmosphericEngine
    PUBLIC
        glm::glm fmt::fmt BulletSoftBody BulletDynamics BulletCollision LinearMath
        lua_static sol2::sol2
        RmlUi::RmlUi box2d::box2d flatbuffers::flatbuffers
)
if (AE_USE_AUDIO)
    target_link_libraries(AtmosphericEngine
        PUBLIC
            raudio
    )
endif()
if(AE_USE_BASIS_UNIVERSAL)
    target_link_libraries(AtmosphericEngine PUBLIC basisu_transcoder)
    target_compile_definitions(AtmosphericEngine PUBLIC AE_USE_BASIS_UNIVERSAL)
endif()
if(AE_USE_EMSCRIPTEN)
    target_compile_definitions(AtmosphericEngine PUBLIC BT_THREADSAFE=1)
    target_compile_options(AtmosphericEngine PRIVATE)
    # -sFETCH=1  : enable emscripten_fetch (XHR + IndexedDB persistence)
    #              used by FileSystem (web platform implementation)
    if(AE_USE_SDL3)
        target_link_options(AtmosphericEngine PRIVATE
            -sWASM=1 -O3
            -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1
            -sFETCH=1)
    elseif(AE_USE_SDL2)
        target_link_options(AtmosphericEngine PRIVATE
            -sWASM=1 -O3 -sUSE_SDL=2
            -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1
            -sFETCH=1)
    else()
        target_link_options(AtmosphericEngine PRIVATE
            -sWASM=1 -O3 -sUSE_GLFW=3
            -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1
            -sFETCH=1)
    endif()
else()
    target_compile_definitions(AtmosphericEngine PUBLIC BT_THREADSAFE=1)
    target_link_libraries(AtmosphericEngine
        PUBLIC
            glad::glad
            spdlog::spdlog
    )
    if(AE_USE_TRACY)
        target_link_libraries(AtmosphericEngine PUBLIC Tracy::TracyClient)
        target_compile_definitions(AtmosphericEngine PUBLIC TRACY_ENABLE TRACY_VERBOSE TRACY_ON_DEMAND)
    endif()
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
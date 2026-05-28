cmake_minimum_required(VERSION 3.19)

option(AE_USE_SDL3 "Use SDL3" ON)
option(AE_USE_SDL2 "Use SDL2" OFF)
# Basis Universal / KTX2 GPU texture compression (auto-enabled for Emscripten builds)
option(AE_USE_BASIS_UNIVERSAL "Enable Basis Universal transcoder for KTX2 GPU-compressed textures" OFF)

# Must be set before project() so the toolchain is loaded correctly.
# The vcpkg wasm32-emscripten community triplet sets VCPKG_CHAINLOAD_TOOLCHAIN_FILE
# automatically, but only when it can locate emcc via EMSCRIPTEN_ROOT or PATH.
# setup-emsdk sets EMSDK but not always EMSCRIPTEN_ROOT, so we provide an explicit
# fallback using EMSDK to ensure the Emscripten toolchain is always found in CI.
if(VCPKG_TARGET_TRIPLET STREQUAL "wasm32-emscripten" AND NOT VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
    if(DEFINED ENV{EMSDK})
        set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE
            "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"
            CACHE STRING "Emscripten toolchain (auto-detected from EMSDK)" FORCE)
    endif()
endif()

project(AtmosphericEngine VERSION 0.1)

# Emscripten-specific overrides (EMSCRIPTEN is set automatically by the toolchain)
if(EMSCRIPTEN)
    # Automatically enable Basis Universal for web builds: KTX2 avoids CPU-side image decoding
    set(AE_USE_BASIS_UNIVERSAL ON CACHE BOOL "Enabled automatically for Emscripten builds" FORCE)
    message(STATUS "Building for Emscripten / WebAssembly")
endif()

if(AE_USE_BASIS_UNIVERSAL)
    message(STATUS "Basis Universal / KTX2 texture compression enabled")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(!MSVC)
    add_compile_options(-Werror=reorder-init-list)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

add_subdirectory(AtmosphericEngine)
add_subdirectory(Example_HelloWorld)
if(NOT EMSCRIPTEN)
    # These demos require glad/spdlog/SDL3 which are desktop-only; skip for web builds
    add_subdirectory(Example_Physics2D)
    add_subdirectory(Example_CSB)
endif()
add_subdirectory(frontends/lua)

cmake_minimum_required(VERSION 3.19)

option(AE_USE_SDL3 "Use SDL3" ON)
option(AE_USE_SDL2 "Use SDL2" OFF)
option(AE_USE_AUDIO "Use audio" ON)
# Basis Universal / KTX2 GPU texture compression (auto-enabled for Emscripten builds)
option(AE_USE_BASIS_UNIVERSAL "Enable Basis Universal transcoder for KTX2 GPU-compressed textures" OFF)

project(AtmosphericEngine VERSION 0.1)

# Emscripten-specific overrides (EMSCRIPTEN is set automatically by the toolchain)
if(EMSCRIPTEN)
    # Automatically enable Basis Universal for web builds: KTX2 avoids CPU-side image decoding
    set(AE_USE_BASIS_UNIVERSAL ON CACHE BOOL "Enabled automatically for Emscripten builds" FORCE)
    # raudio/miniaudio's Emscripten backend requires -sASYNCIFY and -sAUDIO_WORKLET=1
    # which are not yet configured; disable audio automatically for web builds.
    set(AE_USE_AUDIO OFF CACHE BOOL "Disabled automatically for Emscripten builds" FORCE)
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

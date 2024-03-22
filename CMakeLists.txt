cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

# option(USE_MINGW "Use MinGW on Windows" OFF)
option(USE_EMSCRIPTEN "Use Emscripten to build a WebAssembly module" OFF)

if (USE_EMSCRIPTEN)
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE $ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake CACHE STRING "Emscripten toolchain file")
    set(VCPKG_TARGET_TRIPLET wasm32-emscripten)
    message(STATUS "Built with Emscripten")
endif()

project(AtmosphericEngine VERSION 0.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(AtmosphericEngine)
add_subdirectory(Example_MazeWorld)
cmake_minimum_required(VERSION 3.19)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(AtmosphericEngine VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
option(USE_MINGW "Use MinGW on Windows" OFF)
option(USE_EMSCRIPTEN "Use Emscripten to build" OFF)

add_subdirectory(AtmosphericEngine)
add_subdirectory(Example_MazeWorld)

# Copy static assets
# TODO: use add_custom_command instead of file(COPY)
file(COPY assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
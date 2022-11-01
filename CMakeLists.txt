option(USE_MINGW "Use MinGW on Windows" OFF)
option(USE_EMSCRIPTEN "Use Emscripten to build" OFF)

# Platform specific configurations
if(WIN32)
    if(NOT DEFINED ENV{VCPKG_ROOT})
        message(FATAL_ERROR "You need to install VCPKG first!")
    endif()
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "CMake toolchain file" FORCE)
    if(USE_MINGW)
        set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic" CACHE STRING "target triplet" FORCE)
        if(CMAKE_BUILD_TYPE MATCHES "" OR CMAKE_BUILD_TYPE MATCHES "Debug")
            set(CMAKE_IGNORE_PATH "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic/lib") # To prevent from accidentally linking the release build of a library, see https://github.com/microsoft/vcpkg/issues/1626
            set(PKGCONFIG_INSTALL_PREFIX "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic/debug/lib/pkgconfig")
        else()
            set(PKGCONFIG_INSTALL_PREFIX "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic/lib/pkgconfig")
        endif()
    else()
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "target triplet" FORCE)
        if(CMAKE_BUILD_TYPE MATCHES "" OR CMAKE_BUILD_TYPE MATCHES "Debug")
            set(CMAKE_IGNORE_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows/lib") # To prevent from accidentally linking the release build of a library, see https://github.com/microsoft/vcpkg/issues/1626
            set(PKGCONFIG_INSTALL_PREFIX "$ENV{VCPKG_ROOT}/installed/x64-windows/debug/lib/pkgconfig")
        else()
            set(PKGCONFIG_INSTALL_PREFIX "$ENV{VCPKG_ROOT}/installed/x64-windows/lib/pkgconfig")
        endif()
    endif()
    message(VERBOSE "VCPKG install path configured to ${PKGCONFIG_INSTALL_PREFIX} successfully.")
endif()
if(UNIX)
    set(PKGCONFIG_INSTALL_PREFIX "/usr/local/bin")
    message(VERBOSE "VCPKG install path configured to ${PKGCONFIG_INSTALL_PREFIX} successfully.")
endif()


cmake_minimum_required(VERSION 3.10)
project(AtmosphericEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)

aux_source_directory(external SOURCES_EXT)
add_executable(AtmosphericEngine ${PROJECT_SOURCE_DIR}/src/main.cpp ${SOURCES_EXT})
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(Lua REQUIRED)
add_subdirectory(external/sol2)
add_subdirectory(external/glfw)
add_subdirectory(external/bullet3)
add_subdirectory(external/fmt)
add_subdirectory(external/entt)
add_subdirectory(src)

# Link and compile
target_include_directories(AtmosphericEngine PUBLIC src external external/glfw/include external/bullet3/src ${LUA_INCLUDE_DIR})
target_link_libraries(AtmosphericEngine PUBLIC GLEW::GLEW glfw fmt::fmt EnTT::EnTT ${GLFW_LIBRARY} ${GLM_LIBRARY} ${LUA_LIBRARIES} sol2::sol2 BulletSoftBody BulletDynamics BulletCollision LinearMath)

# Copy static files to target directory
file(COPY assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
if(DEFINED ENV{VCPKG_ROOT} AND WIN32)
    if(CMAKE_BUILD_TYPE MATCHES "" OR CMAKE_BUILD_TYPE MATCHES "Debug")
        file(COPY "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic/debug/bin/glew32d.dll" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    else()
        file(COPY "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic/bin/glew32.dll" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endif()
endif()


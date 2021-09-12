# VCPKG + MinGW specific configuration
if(DEFINED ENV{VCPKG_ROOT} AND WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic" CACHE STRING "target triplet" FORCE)
    if(CMAKE_BUILD_TYPE MATCHES "" OR CMAKE_BUILD_TYPE MATCHES "Debug")
        set(CMAKE_IGNORE_PATH "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic/lib") # To prevent from accidentally linking the release build of a library, see https://github.com/microsoft/vcpkg/issues/1626
        set(PKGCONFIG_INSTALL_PREFIX "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic/debug/lib/pkgconfig")
    else()
        set(PKGCONFIG_INSTALL_PREFIX "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic/lib/pkgconfig")
    endif()
endif()

cmake_minimum_required(VERSION 3.10)
project(AtmosphericEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)

aux_source_directory(src SOURCES_0)
aux_source_directory(src/physics SOURCES_1)
aux_source_directory(src/graphics SOURCES_2)
aux_source_directory(src/io SOURCES_3)
aux_source_directory(src/scripting SOURCES_4)
aux_source_directory(src/engine SOURCES_5)
aux_source_directory(external SOURCES_EXT)

find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(Lua REQUIRED)
add_subdirectory(external/sol2)
add_subdirectory(external/glfw)
add_subdirectory(external/bullet3)

# Link and compile
add_executable(AtmosphericEngine ${SOURCES_0} ${SOURCES_1} ${SOURCES_2} ${SOURCES_3} ${SOURCES_4} ${SOURCES_5} ${SOURCES_EXT})
target_include_directories(AtmosphericEngine PUBLIC external ${LUA_INCLUDE_DIR} external/bullet3/src)
target_link_libraries(AtmosphericEngine PUBLIC GLEW::GLEW glfw ${GLFW_LIBRARY} ${GLM_LIBRARY} ${LUA_LIBRARIES} sol2::sol2 BulletSoftBody BulletDynamics BulletCollision LinearMath)

# Copy static files to target directory
file(COPY resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
if(DEFINED ENV{VCPKG_ROOT} AND WIN32)
    if(CMAKE_BUILD_TYPE MATCHES "" OR CMAKE_BUILD_TYPE MATCHES "Debug")
        file(COPY "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic/debug/bin/glew32d.dll" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    else()
        file(COPY "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic/bin/glew32.dll" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endif()
endif()


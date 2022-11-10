option(USE_MINGW "Use MinGW on Windows" OFF)
option(USE_EMSCRIPTEN "Use Emscripten to build" OFF)

cmake_minimum_required(VERSION 3.19)
project(AtmosphericEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)

aux_source_directory(external SOURCES_EXT)
add_executable(AtmosphericEngine ${PROJECT_SOURCE_DIR}/src/main.cpp ${SOURCES_EXT})
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLM REQUIRED)
find_package(Bullet CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_package(sol2 REQUIRED)
find_package(fmt REQUIRED)
add_subdirectory(external/entt)
add_subdirectory(src)

# Link and compile
target_include_directories(AtmosphericEngine PRIVATE src external ${LUA_INCLUDE_DIR} ${BULLET_INCLUDE_DIR})
target_link_libraries(AtmosphericEngine PRIVATE GLEW::GLEW glfw glm::glm fmt::fmt EnTT::EnTT ${LUA_LIBRARIES} sol2::sol2 BulletSoftBody BulletDynamics BulletCollision LinearMath)

# Copy asset files to target directory
file(COPY assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


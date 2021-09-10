cmake_minimum_required(VERSION 3.10)
project(AtmosphericEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/build/)

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
add_subdirectory(external/glfw)
add_subdirectory(external/bullet3)

# Link and compile
add_executable(AtmosphericEngine ${SOURCES_0} ${SOURCES_1} ${SOURCES_2} ${SOURCES_3} ${SOURCES_4} ${SOURCES_5} ${SOURCES_EXT})
target_include_directories(AtmosphericEngine PUBLIC external ${LUA_INCLUDE_DIR} ${BULLET_INCLUDE_DIR})
target_link_libraries(AtmosphericEngine PUBLIC GLEW::GLEW glfw ${GLFW_LIBRARY} ${GLM_LIBRARY} ${LUA_LIBRARY} ${BULLET_SOFTBODY_LIBRARY} ${BULLET_DYNAMICS_LIBRARY} ${BULLET_COLLISION_LIBRARY} ${BULLET_MATH_LIBRARY})

# Copy static files to target directory
file(COPY resources DESTINATION ${EXECUTABLE_OUTPUT_PATH})
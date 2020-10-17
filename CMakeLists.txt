cmake_minimum_required(VERSION 3.10)
project(AtmosphericEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

aux_source_directory(src AtmosphericEngine_SRCS)
aux_source_directory(external AtmosphericEngine_EXTERNAL_SRCS)
add_executable(AtmosphericEngine ${AtmosphericEngine_SRCS} ${AtmosphericEngine_EXTERNAL_SRCS})

# TODO: Should add BulletPhysics/GLEW/GLFW submodules to remove external dependencies
target_include_directories(AtmosphericEngine PUBLIC external /usr/local/include/bullet)
target_link_libraries(AtmosphericEngine PUBLIC GLEW GLFW bulletdynamics linearmath bulletcollision "-framework OpenGL")

# TODO: Copy resources file to target directory
# configure_file(${INPUT_FILE} . COPYONLY)
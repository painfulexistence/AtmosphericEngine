add_executable(MazeWorld main.cpp)
target_link_libraries(MazeWorld PRIVATE AtmosphericEngine)

# Copy static assets
add_custom_command(TARGET MazeWorld POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)

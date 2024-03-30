add_executable(MazeWorld main.cpp)
target_link_libraries(MazeWorld PRIVATE AtmosphericEngine)

add_custom_target(copy_assets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)
add_dependencies(MazeWorld copy_assets)
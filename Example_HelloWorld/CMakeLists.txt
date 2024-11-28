add_executable(HelloWorld main.cpp)
target_link_libraries(HelloWorld PRIVATE AtmosphericEngine)

# add_custom_target(copy_assets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)
# add_dependencies(HelloWorld copy_assets)
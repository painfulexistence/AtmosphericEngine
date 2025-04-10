set(GAME_NAME HelloWorld)
set(GAME_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)

add_executable(${GAME_NAME}
    main.cpp
)
set_target_properties(${GAME_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}
)

target_link_libraries(${GAME_NAME} PRIVATE AtmosphericEngine)
if(USE_EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_compile_options(${GAME_NAME} PRIVATE -pthread)
    target_link_options(${GAME_NAME} PRIVATE -sWASM=1 -O3 -sUSE_GLFW=3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1 -sASSERTIONS=1 -sUSE_PTHREADS=1)
endif()

add_custom_command(
    TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/AtmosphericEngine/default_assets
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}/assets
)

if(EXISTS ${GAME_ASSETS_DIR})
    add_custom_target(copy_${GAME_NAME}_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${GAME_ASSETS_DIR}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}/assets
    )
    add_dependencies(${GAME_NAME} copy_${GAME_NAME}_assets)
endif()
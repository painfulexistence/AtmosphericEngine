set(GAME_NAME Physics2DDemo)
set(GAME_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)

find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(lua REQUIRED)
find_package(Bullet CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(${GAME_NAME}
    main.cpp
)
set_target_properties(${GAME_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${GAME_NAME}
)

target_include_directories(${GAME_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/AtmosphericEngine/include)

target_link_libraries(${GAME_NAME} PRIVATE
    AtmosphericEngine
    glm::glm
    fmt::fmt
    BulletSoftBody
    BulletDynamics
    BulletCollision
    LinearMath
    ${LUA_LIBRARIES}
    sol2
    raudio
    glad::glad
    spdlog::spdlog
    SDL3::SDL3
    box2d
)
if(AE_USE_EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_compile_options(${GAME_NAME} PRIVATE)
    target_link_options(${GAME_NAME} PRIVATE -sWASM=1 -O3 -sUSE_GLFW=3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3=1 -sASSERTIONS=1)
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


find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLM REQUIRED)
find_package(Bullet CONFIG REQUIRED)
find_package(fmt REQUIRED)

add_subdirectory(external/entt)
add_subdirectory(external/lua)
add_subdirectory(external/sol2)

set(SOURCES
    Runtime.cpp
    Application.cpp
    Window.cpp
    Server.cpp
    Console.cpp
    Messagable.cpp
    MessageBus.cpp
    Message.cpp
    ComponentFactory.cpp
    GameObject.cpp
    Impostor.cpp
    light.cpp
    Camera.cpp
    Mesh.cpp
    frustum.cpp
    material.cpp
    Model.cpp
    Shader.cpp
    GraphicsServer.cpp
    texture.cpp
    PhysicsServer.cpp
    DynamicsWorld.cpp
    Input.cpp
    GUI.cpp
    GUIState.cpp
    GUIWindow.cpp
    GUIDocument.cpp
    GUIElement.cpp
    Entity.cpp
    Script.cpp
    IL.cpp
    Lua.cpp
    Utility/File.cpp
)

set(SOURCES_EXT
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_impl_opengl3.cpp
    external/imgui/imgui_impl_glfw.cpp
)

add_library(AtmosphericEngine STATIC ${SOURCES} ${SOURCES_EXT})

target_include_directories(AtmosphericEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${BULLET_INCLUDE_DIR} external/stb external/imgui)
target_link_libraries(AtmosphericEngine PUBLIC GLEW::GLEW glfw glm::glm fmt::fmt EnTT::EnTT BulletSoftBody BulletDynamics BulletCollision LinearMath LUA sol2)
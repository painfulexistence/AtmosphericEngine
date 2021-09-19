set(SOURCES
    src/Graphics/camera.cpp
    src/Graphics/frustum.cpp
    src/Graphics/light.cpp
    src/Graphics/material.cpp
    src/Graphics/mesh.cpp
    src/Graphics/Shader.cpp
    src/Graphics/Renderer.cpp
    src/Graphics/scene.cpp
    src/Graphics/texture.cpp
)
target_sources(AtmosphericEngine PRIVATE ${SOURCES})
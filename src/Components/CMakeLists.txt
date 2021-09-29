set(SOURCES
    src/Components/ComponentFactory.cpp
    src/Components/GameObject.cpp
    src/Components/Impostor.cpp
    src/Components/light.cpp
    src/Components/Camera.cpp
    src/Components/Mesh.cpp
)
target_sources(AtmosphericEngine PRIVATE ${SOURCES})
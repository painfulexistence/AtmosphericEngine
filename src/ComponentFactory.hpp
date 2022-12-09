#pragma once
#include "Globals.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "light.hpp"
#include "Camera.hpp"
#include "Impostor.hpp"
#include "GameObject.hpp"

class GraphicsServer;

class PhysicsServer;

class ComponentFactory
{
public:
    static Light* CreateLight(GameObject*, GraphicsServer*, const LightProps&);

    static Camera* CreateCamera(GameObject*, GraphicsServer*, const CameraProps&);

    static Mesh* CreateMesh(GameObject*, GraphicsServer*, const std::string& modelName);

    static Impostor* CreateImpostor(GameObject*, PhysicsServer*, const std::string& modelName, float mass = 0.0f);
};
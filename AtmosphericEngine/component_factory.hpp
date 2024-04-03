#pragma once
#include "globals.hpp"
#include "renderable.hpp"
#include "mesh.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "impostor.hpp"
#include "game_object.hpp"

class GraphicsServer;

class PhysicsServer;

class ComponentFactory
{
public:
    static Light* CreateLight(GameObject*, GraphicsServer*, const LightProps&);

    static Camera* CreateCamera(GameObject*, GraphicsServer*, const CameraProps&);

    static Renderable* CreateMesh(GameObject*, GraphicsServer*, const std::string& modelName);

    static Impostor* CreateImpostor(GameObject*, PhysicsServer*, const std::string& modelName, float mass = 0.0f);
};
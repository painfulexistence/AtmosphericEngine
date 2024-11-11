#include "component_factory.hpp"
#include "graphics_server.hpp"
#include "physics_server.hpp"

Light* ComponentFactory::CreateLight(GameObject* gameObject,  GraphicsServer* graphics, const LightProps& props)
{
    auto light = new Light(gameObject, props);
    graphics->lights.push_back(light);
    return light;
}

Camera* ComponentFactory::CreateCamera(GameObject* gameObject, GraphicsServer* graphics, const CameraProps& props)
{
    auto camera = new Camera(gameObject, props);
    graphics->cameras.push_back(camera);
    return camera;
}

Renderable* ComponentFactory::CreateMesh(GameObject* gameObject, GraphicsServer* graphics, const std::string& meshName)
{
    if (graphics->MeshList.count(meshName) == 0)
        throw std::runtime_error("Could not find the specified mesh!");

    auto mesh = graphics->MeshList.find(meshName)->second;
    auto renderable = new Renderable(gameObject, mesh);
    graphics->renderables.push_back(renderable);
    return renderable;
}

Impostor* ComponentFactory::CreateImpostor(GameObject* gameObject, PhysicsServer* physics, const std::string& meshName, float mass)
{
    // if (graphics->MeshList.count(meshName) == 0)
    //     throw std::runtime_error("Could not find the specified mesh!");

    // auto mesh = graphics-MeshList.find(meshName)->second;
    // auto impostor =  new Impostor(gameObject, mesh->GetShape(), mass);
    // physics->AddImpostor(impostor);
    // return impostor;
}

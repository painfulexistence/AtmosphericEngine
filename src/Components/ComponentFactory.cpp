#include "Components/ComponentFactory.hpp"
#include "Graphics/GraphicsServer.hpp"
#include "Physics/PhysicsServer.hpp"

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

Mesh* ComponentFactory::CreateMesh(GameObject* gameObject, GraphicsServer* graphics, const std::string& modelName)
{
    if (Model::ModelList.count(modelName) == 0)
        throw std::runtime_error("Could not find the specified model!");

    auto model = Model::ModelList.find(modelName)->second;
    auto mesh = new Mesh(gameObject, model);
    graphics->meshes.push_back(mesh);
    return mesh;
}

Impostor* ComponentFactory::CreateImpostor(GameObject* gameObject, PhysicsServer* physics, const std::string& modelName, float mass)
{
    if (Model::ModelList.count(modelName) == 0)
        throw std::runtime_error("Could not find the specified model!");

    auto model = Model::ModelList.find(modelName)->second;
    auto impostor =  new Impostor(gameObject, model->collisionShape, mass);
    physics->AddImpostor(impostor);
    return impostor;
}

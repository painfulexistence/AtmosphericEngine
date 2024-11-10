#include "game_object.hpp"
#include "component.hpp"
#include "graphics_server.hpp"
#include "physics_server.hpp"
#include "impostor.hpp"

GameObject::GameObject(GraphicsServer* graphics, PhysicsServer* physics)
{
    _graphics = graphics;
    _physics = physics;
}

GameObject::~GameObject()
{
    for (const auto& [name, component] : components)
    {
        delete component;
    }
}

void GameObject::AddComponent(Component* component)
{
    components.insert_or_assign(component->GetName(), component);
    component->gameObject = this;
}

Component* GameObject::GetComponent(std::string name)
{
    auto it = components.find(name);
    if (it == components.end())
        return nullptr;
    else
        return it->second;
}

GameObject* GameObject::AddLight(const LightProps& props)
{
    if (_graphics) {
        auto light = new Light(this, props);
        _graphics->lights.push_back(light);
    }
    return this;
}

GameObject* GameObject::AddCamera(const CameraProps& props)
{
    if (_graphics) {
        auto camera = new Camera(this, props);
        _graphics->cameras.push_back(camera);
    }
    return this;
}

GameObject* GameObject::AddMesh(const std::string& meshName)
{
    if (_graphics) {
        if (Mesh::MeshList.count(meshName) == 0)
            throw std::runtime_error("Could not find the specified mesh!");

        auto mesh = Mesh::MeshList.find(meshName)->second;
        auto renderable = new Renderable(this, mesh);
        _graphics->renderables.push_back(renderable);
    }
    return this;
}

GameObject* GameObject::AddImpostor(const std::string& meshName, float mass)
{
    if (_physics) {
        if (Mesh::MeshList.count(meshName) == 0)
            throw std::runtime_error("Could not find the specified mesh!");

        auto mesh = Mesh::MeshList.find(meshName)->second;
        auto impostor =  new Impostor(this, mesh->GetShape(), mass);
        _physics->AddImpostor(impostor);
    }
    return this;
}

glm::mat4 GameObject::GetModelTransform() const
{
    //return glm::scale(glm::translate(glm::mat4(1.0f), _position),  _scale);
    return _mod;
}

void GameObject::SetModelTransform(glm::mat4 mod)
{
    _mod = mod;
}

glm::mat4 GameObject::GetModelWorldTransform() const
{
    return _m2w;
}

void GameObject::SetModelWorldTransform(glm::mat4 m2w)
{
    this->_m2w = m2w;
}

glm::vec3 GameObject::GetPosition()
{
    return _position;
}

glm::vec3 GameObject::GetRotation()
{
    return _rotation;
}

glm::vec3 GameObject::GetScale()
{
    return _scale;
}

void GameObject::SetPosition(glm::vec3 value)
{
    _position = value;
    _m2w = glm::scale(glm::translate(glm::mat4(1.0f), _position),  _scale);
}

void GameObject::SetRotation(glm::vec3 value)
{
    _rotation = value;
    _m2w = glm::scale(glm::translate(glm::mat4(1.0f), _position),  _scale);
}

void GameObject::SetScale(glm::vec3 value)
{
    _scale = value;
    _m2w = glm::scale(glm::translate(glm::mat4(1.0f), _position),  _scale);
}

glm::mat4 GameObject::GetTransform() const
{
    return _m2w * _mod;
}

glm::vec3 GameObject::GetVelocity()
{
    Impostor* rb = dynamic_cast<Impostor*>(this->GetComponent("Physics"));
    if (rb == nullptr)
        throw std::runtime_error("Impostor not found");

    return rb->GetLinearVelocity();
}

void GameObject::SetVelocity(glm::vec3 value)
{
    Impostor* rb = dynamic_cast<Impostor*>(this->GetComponent("Physics"));
    if (rb == nullptr)
        throw std::runtime_error("Impostor not found");

    rb->SetLinearVelocity(value);
}

void GameObject::ActivatePhyisics()
{
    Impostor* rb = dynamic_cast<Impostor*>(this->GetComponent("Physics"));
    if (rb == nullptr)
        throw std::runtime_error("Impostor not found");

    rb->Activate();
}

void GameObject::FreezePhyisics()
{
    Impostor* rb = dynamic_cast<Impostor*>(this->GetComponent("Physics"));
    if (rb == nullptr)
        throw std::runtime_error("Impostor not found");

    rb->Freeze();
}
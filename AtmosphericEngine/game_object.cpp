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
        if (_graphics->MeshList.count(meshName) == 0)
            throw std::runtime_error("Could not find the specified mesh!");

        auto mesh = _graphics->MeshList.find(meshName)->second;
        auto renderable = new Renderable(this, mesh);
        _graphics->renderables.push_back(renderable);
    }
    return this;
}

GameObject* GameObject::AddImpostor(const std::string& meshName, float mass, glm::vec3 linearFactor, glm::vec3 angularFactor)
{
    if (_graphics && _physics) {
        if (_graphics->MeshList.count(meshName) == 0)
            throw std::runtime_error("Could not find the specified mesh!");

        auto mesh = _graphics->MeshList.find(meshName)->second;
        auto impostor =  new Impostor(this, mesh->GetShape(), mass);
        impostor->SetLinearFactor(linearFactor);
        impostor->SetAngularFactor(angularFactor);
        _physics->AddImpostor(impostor);
    }
    return this;
}

glm::mat4 GameObject::GetLocalTransform() const
{
    return _m2w;
}

void GameObject::SetLocalTransform(glm::mat4 xform)
{
    _m2w = xform;
}

glm::mat4 GameObject::GetObjectTransform() const
{
    return _w2w;
}

// FIXME: This is not working properly
void GameObject::SetObjectTransform(glm::mat4 xform)
{
    this->_w2w = xform;
    _position = glm::vec3(_w2w[3]);
    _scale = glm::vec3(glm::length(_w2w[0]), glm::length(_w2w[1]), glm::length(_w2w[2]));

    // Impostor* rb = dynamic_cast<Impostor*>(this->GetComponent("Physics"));
    // if (rb)
    //     rb->SetTransform(_rotation, _position);
}

void GameObject::SyncObjectTransform(glm::mat4 xform)
{
    this->_w2w = xform;
    _position = glm::vec3(_w2w[3]);
    _scale = glm::vec3(glm::length(_w2w[0]), glm::length(_w2w[1]), glm::length(_w2w[2]));
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
    _w2w = glm::scale(glm::translate(glm::mat4(1.0f), _position),  _scale);
}

void GameObject::SetRotation(glm::vec3 value)
{
    _rotation = value;
    _w2w = glm::scale(glm::translate(glm::mat4(1.0f), _position),  _scale);
}

void GameObject::SetScale(glm::vec3 value)
{
    _scale = value;
    _w2w = glm::scale(glm::translate(glm::mat4(1.0f), _position),  _scale);
}

glm::mat4 GameObject::GetTransform() const
{
    return _w2w * _m2w;
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
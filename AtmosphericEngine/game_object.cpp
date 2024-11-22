#include "game_object.hpp"
#include "component.hpp"
#include "graphics_server.hpp"
#include "physics_server.hpp"
#include "impostor.hpp"

GameObject::GameObject(GraphicsServer* graphics, PhysicsServer* physics, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) : _graphics(graphics), _physics(physics)
{
    _position = position;
    _rotation = rotation;
    _scale = scale;
    _w2w = glm::scale(glm::translate(glm::mat4(1.0f), position), scale);
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

void GameObject::RemoveComponent(Component* component)
{
    components.erase(component->GetName());
}

Component* GameObject::GetComponent(std::string name) const
{
    auto it = components.find(name);
    if (it == components.end())
        return nullptr;
    else
        return it->second;
}

// Shortcut for adding light component
GameObject* GameObject::AddLight(const LightProps& props)
{
    if (_graphics) {
        auto light = _graphics->CreateLight(this, props);
    }
    return this;
}

// Shortcut for adding camera component
GameObject* GameObject::AddCamera(const CameraProps& props)
{
    if (_graphics) {
        auto camera = _graphics->CreateCamera(this, props);
    }
    return this;
}

// Shortcut for adding renderable component
GameObject* GameObject::AddRenderable(const std::string& meshName)
{
    if (_graphics) {
        auto mesh = _graphics->GetMesh(meshName);
        auto renderable = _graphics->CreateRenderable(this, mesh);
    }
    return this;
}

// Shortcut for adding impostor component
GameObject* GameObject::AddImpostor(const std::string& meshName, float mass, glm::vec3 linearFactor, glm::vec3 angularFactor)
{
    if (_graphics && _physics) {
        auto mesh = _graphics->GetMesh(meshName);
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

void GameObject::SetObjectTransform(glm::mat4 xform)
{
    _w2w = xform;
    _position = glm::vec3(_w2w[3]);
    _scale = glm::vec3(glm::length(_w2w[0]), glm::length(_w2w[1]), glm::length(_w2w[2]));

    Impostor* rb = dynamic_cast<Impostor*>(this->GetComponent("Physics"));
    if (rb)
        rb->SetWorldTransform(_position, _rotation);
}

void GameObject::SyncObjectTransform(glm::mat4 xform)
{
    _w2w = xform;
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

    Impostor* rb = dynamic_cast<Impostor*>(GetComponent("Physics"));
    if (rb)
        rb->SetWorldTransform(_position, _rotation);
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
    Impostor* rb = dynamic_cast<Impostor*>(GetComponent("Physics"));
    if (rb == nullptr)
        throw std::runtime_error("Impostor not found");

    return rb->GetLinearVelocity();
}

void GameObject::SetVelocity(glm::vec3 value)
{
    Impostor* rb = dynamic_cast<Impostor*>(GetComponent("Physics"));
    if (rb == nullptr)
        throw std::runtime_error("Impostor not found");

    rb->SetLinearVelocity(value);
}

void GameObject::SetPhysicsActivated(bool value)
{
    Impostor* rb = dynamic_cast<Impostor*>(GetComponent("Physics"));
    if (rb == nullptr)
        throw std::runtime_error("Impostor not found");

    if (value) {
        rb->Act();
    } else {
        rb->Stop();
    }
}
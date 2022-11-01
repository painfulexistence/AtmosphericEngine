#include "GameObject.hpp"
#include "Component.hpp"

GameObject::GameObject()
{

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
#pragma once
#include "Globals.hpp"

class Component;

class GameObject
{
public:
    GameObject* parent = nullptr;
    std::map<std::string, Component*> components;

    GameObject();

    ~GameObject();

    void AddComponent(Component* component);

    Component* GetComponent(std::string name);

    glm::mat4 GetModelTransform() const;

    void SetModelTransform(glm::mat4 mod);

    glm::mat4 GetModelWorldTransform() const;

    void SetModelWorldTransform(glm::mat4 m2w);

    glm::vec3 GetPosition();

    glm::vec3 GetRotation();

    glm::vec3 GetScale();

    void SetPosition(glm::vec3 value);

    void SetRotation(glm::vec3 value);

    void SetScale(glm::vec3 value);

    glm::mat4 GetTransform() const; // World space

private:
    glm::mat4 _mod = glm::mat4(1.0f);
    glm::mat4 _m2w = glm::mat4(1.0f);
    glm::vec3 _position = glm::vec3(0, 0, 0);
    glm::vec3 _rotation = glm::vec3(0, 0, 0);
    glm::vec3 _scale = glm::vec3(1, 1, 1);
};
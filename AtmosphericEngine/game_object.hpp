#pragma once
#include "globals.hpp"

class Component;

class LightProps;

class CameraProps;

class GraphicsServer;

class PhysicsServer;

class GameObject
{
public:
    GameObject* parent = nullptr;
    std::map<std::string, Component*> components;
    bool isActive = true;

    GameObject(GraphicsServer* graphics = nullptr, PhysicsServer* physics = nullptr, glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

    ~GameObject();

    void AddComponent(Component* component);

    void RemoveComponent(Component* component);

    Component* GetComponent(std::string name) const;

    GameObject* AddLight(const LightProps&);

    GameObject* AddCamera(const CameraProps&);

    GameObject* AddRenderable(const std::string& meshName);

    GameObject* AddImpostor(const std::string& meshName, float mass = 0.0f, glm::vec3 linearFactor = glm::vec3(1.0f), glm::vec3 angularFactor = glm::vec3(1.0f));

    glm::mat4 GetLocalTransform() const;

    void SetLocalTransform(glm::mat4 xform);

    glm::mat4 GetObjectTransform() const;

    void SetObjectTransform(glm::mat4 xform);

    void SyncObjectTransform(glm::mat4 xform);

    glm::vec3 GetPosition();

    glm::vec3 GetRotation();

    glm::vec3 GetScale();

    void SetPosition(glm::vec3 value);

    void SetRotation(glm::vec3 value);

    void SetScale(glm::vec3 value);

    glm::mat4 GetTransform() const; // World space

    glm::vec3 GetVelocity();

    void SetVelocity(glm::vec3 value);

    void SetActive(bool value) { isActive = value; }

    void SetPhysicsActivated(bool value);

private:
    GraphicsServer* _graphics = nullptr;
    PhysicsServer* _physics = nullptr;
    glm::mat4 _m2w = glm::mat4(1.0f);
    glm::mat4 _w2w = glm::mat4(1.0f);
    glm::vec3 _position = glm::vec3(0, 0, 0);
    glm::vec3 _rotation = glm::vec3(0, 0, 0);
    glm::vec3 _scale = glm::vec3(1, 1, 1);
    glm::vec3 _velocity = glm::vec3(0, 0, 0);
    glm::vec3 _angularVelocity = glm::vec3(0, 0, 0);
};
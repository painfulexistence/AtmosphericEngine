#pragma once
#include "globals.hpp"
#include "component.hpp"

class GameObject;

class Mesh;

class Renderable : public Component
{
public:
    Renderable(GameObject* gameObject, Mesh* mesh);

    ~Renderable();

    std::string GetName() const override;

    Mesh* mesh = nullptr;
private:
    //Material* material;
    //btCollisionShape* collider;
};
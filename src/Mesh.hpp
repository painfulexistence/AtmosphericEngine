#pragma once
#include "Globals.hpp"
#include "Component.hpp"

class GameObject;

class Model;

class Mesh : public Component
{
public:
    Model* model = nullptr;

    Mesh(GameObject* gameObject, Model* model);

    ~Mesh();

    std::string GetName() const override;

private:
    //Material* material;
    //btCollisionShape* collider;
};
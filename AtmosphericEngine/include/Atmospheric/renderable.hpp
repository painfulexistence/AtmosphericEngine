#pragma once
#include "globals.hpp"
#include "component.hpp"

class GameObject;

class Mesh;

class Material;

class Renderable : public Component
{
public:
    Renderable(GameObject* gameObject, Mesh* mesh);

    ~Renderable();

    std::string GetName() const override;

    Mesh* GetMesh() const;

    void SetMesh(Mesh* mesh);

    Material* GetMaterial() const;

    void SetMaterial(Material* material);

private:
    Mesh* _mesh = nullptr;
    Material* _material = nullptr;
};
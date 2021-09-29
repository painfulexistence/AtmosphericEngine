#include "Components/Mesh.hpp"
#include "Components/GameObject.hpp"
#include "Graphics/Model.hpp"

Mesh::Mesh(GameObject* gameObject, Model* model)
{
    this->model = model;
    
    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
}

Mesh::~Mesh()
{

}

std::string Mesh::GetName() const
{
    return std::string("Mesh");
}
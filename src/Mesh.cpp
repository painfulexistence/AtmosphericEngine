#include "Mesh.hpp"
#include "GameObject.hpp"
#include "Model.hpp"

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
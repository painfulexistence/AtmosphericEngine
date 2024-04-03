#include "renderable.hpp"
#include "game_object.hpp"
#include "mesh.hpp"

Renderable::Renderable(GameObject* gameObject, Mesh* mesh)
{
    this->mesh = mesh;

    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
}

Renderable::~Renderable()
{

}

std::string Renderable::GetName() const
{
    return std::string("Mesh");
}
#include "renderable.hpp"
#include "game_object.hpp"
#include "mesh.hpp"
#include "material.hpp"

Renderable::Renderable(GameObject* gameObject, Mesh* mesh) {
    this->_mesh = mesh;

    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
}

Renderable::~Renderable() {

}

std::string Renderable::GetName() const {
    return std::string("Mesh");
}

Mesh* Renderable::GetMesh() const {
    return _mesh;
}

void Renderable::SetMesh(Mesh* mesh) {
    _mesh = mesh;
}

Material* Renderable::GetMaterial() const {
    if (_material) {
        return _material;
    } else {
        return _mesh->GetMaterial();
    }
}

void Renderable::SetMaterial(Material* material) {
    _material = material;
}
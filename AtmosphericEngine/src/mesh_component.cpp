#include "mesh_component.hpp"
#include "game_object.hpp"
#include "mesh.hpp"
#include "material.hpp"

MeshComponent::MeshComponent(GameObject* gameObject, Mesh* mesh) {
    this->_mesh = mesh;

    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
}

MeshComponent::~MeshComponent() {

}

std::string MeshComponent::GetName() const {
    return std::string("Drawable");
}

Mesh* MeshComponent::GetMesh() const {
    return _mesh;
}

void MeshComponent::SetMesh(Mesh* mesh) {
    _mesh = mesh;
}

Material* MeshComponent::GetMaterial() const {
    if (_material) {
        return _material;
    } else {
        return _mesh->GetMaterial();
    }
}

void MeshComponent::SetMaterial(Material* material) {
    _material = material;
}
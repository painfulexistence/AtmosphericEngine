#include "drawable_2d.hpp"
#include "component.hpp"
#include "game_object.hpp"

Drawable2D::Drawable2D(GameObject* gameObject) {
    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
}

std::string Drawable2D::GetName() const {
    return std::string("Drawable2D");
}
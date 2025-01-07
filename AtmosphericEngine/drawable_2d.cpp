#include "drawable_2d.hpp"
#include "component.hpp"
#include "game_object.hpp"

Drawable2D::Drawable2D(GameObject* gameObject, const Drawable2DProps& props) {
    _size = props.size;
    _color = props.color;
    _pivot = props.pivot;
    _textureID = props.textureID;

    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
}

std::string Drawable2D::GetName() const {
    return std::string("Drawable2D");
}
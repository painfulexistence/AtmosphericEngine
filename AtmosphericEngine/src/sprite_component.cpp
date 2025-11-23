#include "sprite_component.hpp"
#include "component.hpp"
#include "game_object.hpp"

SpriteComponent::SpriteComponent(GameObject* gameObject, const SpriteProps& props) {
    _size = props.size;
    _color = props.color;
    _pivot = props.pivot;
    _textureID = props.textureID;

    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
}

std::string SpriteComponent::GetName() const {
    return std::string("SpriteComponent");
}
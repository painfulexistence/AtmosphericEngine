#include "sprite_component.hpp"
#include "application.hpp"
#include "component.hpp"
#include "game_object.hpp"

SpriteComponent::SpriteComponent(GameObject* gameObject, const SpriteProps& props) {
    _size = props.size;
    _color = props.color;
    _pivot = props.pivot;
    _textureID = props.textureID;
    _layer = props.layer;
}

std::string SpriteComponent::GetName() const {
    return std::string("SpriteComponent");
}

void SpriteComponent::OnAttach() {
    gameObject->GetApp()->GetGraphicsServer()->RegisterSprite(this);
}

void SpriteComponent::OnDetach() {
}

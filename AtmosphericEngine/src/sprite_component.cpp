#include "sprite_component.hpp"
#include "application.hpp"
#include "batch_renderer_2d.hpp"
#include "canvas_drawable.hpp"
#include "game_object.hpp"
#include "graphics_server.hpp"
#include "renderer.hpp"

SpriteComponent::SpriteComponent(GameObject* gameObject, const SpriteProps& props) : CanvasDrawable(gameObject) {
    _size = props.size;
    _color = props.color;
    _pivot = props.pivot;
    _textureID = props.textureID;
    _layer = props.layer;
    _uvMin = glm::vec2(0.0f, 0.0f);
    _uvMax = glm::vec2(1.0f, 1.0f);
}

std::string SpriteComponent::GetName() const {
    return std::string("SpriteComponent");
}

void SpriteComponent::OnAttach() {
    gameObject->GetApp()->GetGraphicsServer()->RegisterCanvasDrawable(this);
}

void SpriteComponent::OnDetach() {
}

void SpriteComponent::Draw(BatchRenderer2D* renderer) {
    glm::vec3 pos = gameObject->GetPosition();
    glm::vec3 rot = gameObject->GetRotation();
    glm::vec3 scale = gameObject->GetScale();

    glm::vec2 size = _size * glm::vec2(scale.x, scale.y);

    // Transform
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos);
    transform = glm::rotate(transform, rot.z, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec2 pivotOffset = (glm::vec2(0.5f, 0.5f) - _pivot) * size;
    transform = glm::translate(transform, glm::vec3(pivotOffset, 0.0f));

    transform = glm::scale(transform, glm::vec3(size.x, size.y, 1.0f));

    glm::vec2 uvs[4] = {
        { _uvMin.x, _uvMin.y },// BL
        { _uvMax.x, _uvMin.y },// BR
        { _uvMax.x, _uvMax.y },// TR
        { _uvMin.x, _uvMax.y }// TL
    };

    renderer->DrawQuad(transform, _textureID, uvs, _color, (int)_layer);
}

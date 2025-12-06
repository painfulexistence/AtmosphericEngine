#include "sprite_3d_component.hpp"
#include "application.hpp"
#include "batch_renderer_2d.hpp"
#include "camera_component.hpp"
#include "game_object.hpp"
#include "graphics_server.hpp"

Sprite3DComponent::Sprite3DComponent(GameObject* gameObject, const Sprite3DProps& props)
    : CanvasDrawable(gameObject) {
    _size = props.size;
    _color = props.color;
    _pivot = props.pivot;
    _textureID = props.textureID;
    _billboardMode = props.billboardMode;
}

std::string Sprite3DComponent::GetName() const {
    return std::string("Sprite3DComponent");
}

void Sprite3DComponent::OnAttach() {
    gameObject->GetApp()->GetGraphicsServer()->RegisterCanvasDrawable(this);
}

void Sprite3DComponent::OnDetach() {
}

glm::mat4 Sprite3DComponent::CalculateBillboardMatrix(const glm::vec3& position, const glm::vec3& cameraPosition) const {
    glm::mat4 billboard = glm::mat4(1.0f);

    switch (_billboardMode) {
    case BillboardMode::ViewPoint: {
        // Spherical billboard: face camera position exactly
        glm::vec3 look = glm::normalize(cameraPosition - position);
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

        // Handle edge case: camera directly above/below
        if (glm::abs(glm::dot(look, worldUp)) > 0.999f) {
            worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        glm::vec3 right = glm::normalize(glm::cross(worldUp, look));
        glm::vec3 up = glm::cross(look, right);

        billboard[0] = glm::vec4(right, 0.0f);
        billboard[1] = glm::vec4(up, 0.0f);
        billboard[2] = glm::vec4(look, 0.0f);
        billboard[3] = glm::vec4(position, 1.0f);
        break;
    }
    case BillboardMode::ViewPlane: {
        // Cylindrical billboard: face camera but keep Y-axis locked
        glm::vec3 look = cameraPosition - position;
        look.y = 0.0f;// Lock Y-axis

        if (glm::length(look) < 0.001f) {
            // Camera is directly above/below, use default forward
            look = glm::vec3(0.0f, 0.0f, 1.0f);
        } else {
            look = glm::normalize(look);
        }

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(up, look));

        billboard[0] = glm::vec4(right, 0.0f);
        billboard[1] = glm::vec4(up, 0.0f);
        billboard[2] = glm::vec4(look, 0.0f);
        billboard[3] = glm::vec4(position, 1.0f);
        break;
    }
    case BillboardMode::None:
    default:
        // No billboarding, just translation
        billboard = glm::translate(glm::mat4(1.0f), position);
        break;
    }

    return billboard;
}

void Sprite3DComponent::Draw(BatchRenderer2D* renderer) {
    glm::vec3 pos = gameObject->GetPosition();
    glm::vec3 scale = gameObject->GetScale();

    // Get camera for billboard calculation
    auto* graphics = gameObject->GetApp()->GetGraphicsServer();
    auto* camera = graphics->GetMainCamera();

    glm::mat4 transform;

    if (_billboardMode != BillboardMode::None && camera) {
        glm::vec3 cameraPos = camera->GetEyePosition();
        transform = CalculateBillboardMatrix(pos, cameraPos);
    } else {
        // Use object's own rotation
        glm::vec3 rot = gameObject->GetRotation();
        transform = glm::translate(glm::mat4(1.0f), pos);
        transform = glm::rotate(transform, rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // Apply size with scale
    glm::vec2 finalSize = _size * glm::vec2(scale.x, scale.y);

    // Apply pivot offset
    glm::vec2 pivotOffset = (glm::vec2(0.5f, 0.5f) - _pivot) * finalSize;
    transform = glm::translate(transform, glm::vec3(pivotOffset, 0.0f));

    // Apply scale
    transform = glm::scale(transform, glm::vec3(finalSize.x, finalSize.y, 1.0f));

    // Setup UVs
    glm::vec2 uvs[4] = {
        { _uvMin.x, _uvMin.y },// BL
        { _uvMax.x, _uvMin.y },// BR
        { _uvMax.x, _uvMax.y },// TR
        { _uvMin.x, _uvMax.y } // TL
    };

    renderer->DrawQuad(transform, _textureID, uvs, _color);
}

#include "transform_component.hpp"
#include "game_object.hpp"

TransformComponent::TransformComponent(GameObject* owner, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
    gameObject = owner;
    _position = position;
    _rotation = rotation;
    _scale = scale;
    UpdateTransform();
}

void TransformComponent::SetPosition(const glm::vec3& position) {
    _position = position;
    UpdateTransform();
}

void TransformComponent::SetRotation(const glm::vec3& rotation) {
    _rotation = rotation;
    UpdateTransform();
}

void TransformComponent::SetScale(const glm::vec3& scale) {
    _scale = scale;
    UpdateTransform();
}

glm::mat4 TransformComponent::GetLocalTransform() const {
    return _localToWorld;
}

glm::mat4 TransformComponent::GetWorldTransform() const {
    if (gameObject && gameObject->parent) {
        return gameObject->parent->GetTransform() * _localToWorld;
    }
    return _localToWorld;
}

void TransformComponent::SetLocalTransform(const glm::mat4& transform) {
    _localToWorld = transform;
    UpdatePositionRotationScale();
}

void TransformComponent::SetWorldTransform(const glm::mat4& transform) {
    if (gameObject && gameObject->parent) {
        glm::mat4 parentTransform = gameObject->parent->GetTransform();
        _localToWorld = glm::inverse(parentTransform) * transform;
    } else {
        _localToWorld = transform;
    }
    UpdatePositionRotationScale();
}

void TransformComponent::SyncWorldTransform(const glm::mat4& transform) {
    SetWorldTransform(transform);
}

// Update local matrix from P/R/S
void TransformComponent::UpdateTransform() {
    // _rotation is stored in degrees (user-friendly), convert to radians for glm::quat
    glm::vec3 rotationRad = glm::radians(_rotation);
    _localToWorld = glm::translate(glm::mat4(1.0f), _position) * glm::mat4_cast(glm::quat(rotationRad))
                    * glm::scale(glm::mat4(1.0f), _scale);
}

// Update P/R/S from local matrix
void TransformComponent::UpdatePositionRotationScale() {
    _position = glm::vec3(_localToWorld[3]);
    // glm::eulerAngles returns radians, convert to degrees for user-friendly storage
    _rotation = glm::degrees(glm::eulerAngles(glm::quat_cast(glm::mat3(_localToWorld))));
    _scale = glm::vec3(glm::length(_localToWorld[0]), glm::length(_localToWorld[1]), glm::length(_localToWorld[2]));
}

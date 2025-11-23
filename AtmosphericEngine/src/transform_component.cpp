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
    return _worldToWorld;
}

void TransformComponent::SetLocalTransform(const glm::mat4& transform) {
    _localToWorld = transform;
}

void TransformComponent::SetWorldTransform(const glm::mat4& transform) {
    _worldToWorld = transform;
    UpdatePositionRotationScale();
}

void TransformComponent::SyncWorldTransform(const glm::mat4& transform) {
    _worldToWorld = transform;
    UpdatePositionRotationScale();
}

void TransformComponent::UpdateTransform() {
    _worldToWorld = glm::translate(glm::mat4(1.0f), _position) * glm::mat4_cast(glm::quat(_rotation))
                    * glm::scale(glm::mat4(1.0f), _scale);
}

void TransformComponent::UpdatePositionRotationScale() {
    _position = glm::vec3(_worldToWorld[3]);
    _rotation = glm::eulerAngles(glm::quat_cast(glm::mat3(_worldToWorld)));
    _scale = glm::vec3(glm::length(_worldToWorld[0]), glm::length(_worldToWorld[1]), glm::length(_worldToWorld[2]));
}

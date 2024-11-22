#include "camera.hpp"
#include "game_object.hpp"

static const float maxVAngle = PI / 2.0f - 0.01f;
static const float minVAngle = -PI / 2.0f + 0.01f;

Camera::Camera(GameObject* gameObject, const CameraProps& props)
{
    _fov = props.fieldOfView;
    _aspectRatio = props.aspectRatio;
    _nearZ = props.nearClip;
    _farZ = props.farClip;
    _eyeOffset = props.eyeOffset;
    _vhAngle = glm::vec2(props.verticalAngle, props.horizontalAngle);

    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
}

std::string Camera::GetName() const
{
    return std::string("Camera");
}

glm::vec3 Camera::GetEyePosition()
{
    glm::mat4 xform = gameObject->GetObjectTransform();
    return glm::vec3(xform * glm::vec4(_eyeOffset, 1.0));
}

glm::vec3 Camera::GetEyeDirection()
{
    float vAngle = _vhAngle.x, hAngle = _vhAngle.y;
    return glm::vec3(glm::cos(vAngle) * glm::cos(hAngle), glm::sin(vAngle), glm::sin(hAngle) * glm::cos(vAngle));
}

glm::mat4 Camera::GetViewMatrix()
{
    glm::vec3 eyePos = GetEyePosition();
    return glm::lookAt(eyePos, eyePos + GetEyeDirection(), glm::vec3(0, 1, 0));
}

glm::mat4 Camera::GetProjectionMatrix()
{
    return glm::perspective(_fov, _aspectRatio, _nearZ, _farZ);
}

glm::vec3 Camera::GetMoveVector(Axis axis)
{
    glm::vec3 dir = GetEyeDirection();
    switch (axis) {
    case BACK:
    case FRONT:
        return (float)CAMERA_SPEED * glm::normalize(glm::vec3(dir.x, 0, dir.z));
    case RIGHT:
    case LEFT:
        return (float)CAMERA_SPEED * glm::normalize(glm::cross(dir, glm::vec3(0, 1, 0)));
    case UP:
    case DOWN:
        return (float)CAMERA_VERTICAL_SPEED * glm::vec3(0, 1, 0);
    default:
        throw std::runtime_error("Invalid camera move axis");
    }
}

void Camera::Yaw(float angleOffset)
{
    _vhAngle.y += angleOffset;
}

void Camera::Pitch(float angleOffset)
{
    _vhAngle.x = std::max(minVAngle, std::min(maxVAngle, _vhAngle.x + angleOffset));
}


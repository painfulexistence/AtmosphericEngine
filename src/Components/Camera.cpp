#include "Components/Camera.hpp"
#include "Components/GameObject.hpp"

static const float maxVAngle = PI / 2.0f - 0.01f;
static const float minVAngle = -PI / 2.0f + 0.01f;

static glm::vec3 CalculateDirection(float vAngle, float hAngle)
{
    return glm::vec3(glm::cos(vAngle) * glm::cos(hAngle), glm::sin(vAngle), glm::sin(hAngle) * glm::cos(vAngle));
}

Camera::Camera(GameObject* gameObject, const CameraProps& props)
{
    fov = props.fieldOfView;
    aspectRatio = props.aspectRatio;
    nearZ = props.nearClipPlane;
    farZ = props.farClipPlane;
    _eyeOffset = props.eyeOffset;
    _vhAngle = glm::vec2(props.verticalAngle, props.horizontalAngle);
    
    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
}

std::string Camera::GetName() const
{
    return std::string("Camera");
}

glm::vec3 Camera::GetEye(const glm::mat4& cameraTransform)
{
    return glm::vec3(cameraTransform * glm::vec4(_eyeOffset, 1.0));
}

glm::mat4 Camera::GetViewMatrix(const glm::mat4& cameraTransform) 
{
    glm::vec3 eyePos = GetEye(cameraTransform);
    return glm::lookAt(eyePos, eyePos + CalculateDirection(_vhAngle.x, _vhAngle.y), glm::vec3(0, 1, 0));
}

glm::mat4 Camera::GetProjectionMatrix()
{
    return glm::perspective(fov, aspectRatio, nearZ, farZ);
}

glm::vec3 Camera::CreateLinearVelocity(Axis axis)
{
    glm::vec3 dir;
    switch (axis)
    {
        case BACK:
        case FRONT:
            dir = CalculateDirection(_vhAngle.x, _vhAngle.y);
            return (float)CAMERA_SPEED * glm::normalize(glm::vec3(dir.x, 0, dir.z));
        case RIGHT:
        case LEFT:
            dir = CalculateDirection(_vhAngle.x, _vhAngle.y);
            return (float)CAMERA_SPEED * glm::normalize(glm::cross(dir, glm::vec3(0, 1, 0)));
        case UP:
        case DOWN:
            return (float)CAMERA_VERTICAL_SPEED * glm::vec3(0, 1, 0);
        default:
            return glm::vec3(0);
    }
}

void Camera::yaw(float angleOffset) 
{
    _vhAngle.y += angleOffset;
}

void Camera::pitch(float angleOffset)
{
    _vhAngle.x = std::max(minVAngle, std::min(maxVAngle, _vhAngle.x + angleOffset));
}


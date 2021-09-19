#include "Graphics/camera.hpp"
#include "Physics.hpp"

static const float maxVAngle = PI / 2.0f - 0.01f;
static const float minVAngle = -PI / 2.0f + 0.01f;

Camera::Camera(sol::table t)
{
    _eyeOffset = glm::vec3(t["eyeOffset"][1], t["eyeOffset"][2], t["eyeOffset"][3]);
    _vhAngle = glm::vec2((float)t.get_or("vAngle", 0.f), t.get_or("hAngle", 0.f));

    fov = (float)t.get_or("fov", glm::radians(60.f));
    aspectRatio = (float)t.get_or("aspectRatio", (float)SCREEN_W / (float)SCREEN_H);
    nearZ = (float)t.get_or("nearZ", 0.1f);
    farZ = (float)t.get_or("farZ", 2000.0f);
}

Camera::Camera(CameraProperties props) 
{
    _eyeOffset = props.eyeOffset;
    _vhAngle = glm::vec2(props.vAngle, props.hAngle);

    fov = props.fov;
    aspectRatio = props.aspectRatio;
    nearZ = props.nearClipPlane;
    farZ = props.farClipPlane;
}

static glm::vec3 CalculateDirection(float vAngle, float hAngle)
{
    return glm::vec3(glm::cos(vAngle) * glm::cos(hAngle), glm::sin(vAngle), glm::sin(hAngle) * glm::cos(vAngle));
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


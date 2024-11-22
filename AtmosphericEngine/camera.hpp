#pragma once
#include "globals.hpp"
#include "component.hpp"

struct CameraProps
{
    float fieldOfView = 45.0f;
    float aspectRatio = 0.75f;
    float nearClip = 0.1f;
    float farClip = 100.0f;
    float verticalAngle = 0.0f;
    float horizontalAngle = 0.0f;
    glm::vec3 eyeOffset = glm::vec3(0.0f);
};

class Camera : public Component
{
public:

    Camera(GameObject* gameObject, const CameraProps& props);

    std::string GetName() const override;

    glm::vec3 GetEyePosition();

    glm::vec3 GetEyeDirection();

    glm::mat4 GetViewMatrix();

    glm::mat4 GetProjectionMatrix();

    glm::vec3 GetMoveVector(Axis);

    void Yaw(float);

    void Pitch(float);

private:
    float _fov;
    float _aspectRatio;
    float _nearZ;
    float _farZ;
    glm::vec3 _eyeOffset;
    glm::vec2 _vhAngle;
};

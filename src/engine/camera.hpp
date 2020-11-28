#pragma once
#include "../common.hpp"
#include "entity.hpp"
#include "../physics/BulletMain.h"

struct CameraProperties
{
    glm::vec3 origin = glm::vec3(0, 0, 0);
    float fov = glm::radians(60.0f);
    float aspectRatio = 4.f / 3.f;
    float nearClipPlane = 0.1f;
    float farClipPlane = 1000.f;
};

class Camera : public Entity
{
private:
    glm::vec3 _eyeOffset;
    glm::vec2 _vhAngle;
    glm::mat4 _projection;
    glm::mat4 _view;

public:
    Camera(CameraProperties props = CameraProperties(), glm::vec2 vhAngle = glm::vec2(0, 0));

    glm::vec3 GetEye(const glm::mat4&);

    glm::mat4 GetViewMatrix(const glm::mat4&);

    glm::mat4 GetProjectionMatrix();

    void yaw(float);

    void pitch(float);

    glm::vec3 CreateLinearVelocity(Axis);
};

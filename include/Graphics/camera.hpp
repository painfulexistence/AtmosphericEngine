#pragma once
#include "Globals.hpp"
#include "entity.hpp"
#include "Physics.hpp"
#include "Scripting/Lua.hpp"

struct CameraProperties
{
    glm::vec3 eyeOffset = glm::vec3(0, 0, 0);
    float fov = glm::radians(60.0f);
    float aspectRatio = 4.f / 3.f;
    float nearClipPlane = 0.1f;
    float farClipPlane = 1000.f;
    float vAngle = 0.f;
    float hAngle = 0.f;
};

class Camera : public Entity
{
    glm::vec3 _eyeOffset = glm::vec3(0, 0, 0);
    glm::vec2 _vhAngle = glm::vec2(0, 0);

public:
    float fov = glm::radians(60.0f);
    float aspectRatio = (float)SCREEN_W / (float)SCREEN_H;
    float nearZ = 0.1f;
    float farZ = 1000.f;

    Camera(sol::table);

    Camera(CameraProperties props);

    glm::vec3 GetEye(const glm::mat4&);

    glm::mat4 GetViewMatrix(const glm::mat4&);

    glm::mat4 GetProjectionMatrix();

    void yaw(float);

    void pitch(float);

    glm::vec3 CreateLinearVelocity(Axis);
};

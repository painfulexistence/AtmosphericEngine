#pragma once
#include "Globals.hpp"
#include "lua.hpp"
#include "Component.hpp"

struct CameraProps
{
    CameraProps(sol::table data)
    {
        this->fieldOfView = (float)data.get_or("field_of_view", glm::radians(60.f));
        this->aspectRatio = (float)data.get_or("aspect_ratio", 4.f / 3.f);
        this->nearClipPlane = (float)data.get_or("near_clip_plane", 0.1f);
        this->farClipPlane = (float)data.get_or("far_clip_plane", 1000.0f);
        this->verticalAngle = (float)data.get_or("vertical_angle", 0);
        this->horizontalAngle = (float)data.get_or("horizontal_angle", 0);
        this->eyeOffset = glm::vec3(
            (float)data.get_or("eye_offset.x", 0),
            (float)data.get_or("eye_offset.y", 0),
            (float)data.get_or("eye_offset.z", 0)
        );
    };
    float fieldOfView;
    float aspectRatio;
    float nearClipPlane;
    float farClipPlane;
    float verticalAngle;
    float horizontalAngle;
    glm::vec3 eyeOffset;
};

class Camera : public Component
{
    glm::vec3 _eyeOffset;
    glm::vec2 _vhAngle;

public:
    float fov;
    float aspectRatio;
    float nearZ;
    float farZ;

    Camera(GameObject* gameObject, const CameraProps& props);

    std::string GetName() const override;

    glm::vec3 GetEye(const glm::mat4&);

    glm::mat4 GetViewMatrix(const glm::mat4&);

    glm::mat4 GetProjectionMatrix();

    void yaw(float);

    void pitch(float);

    glm::vec3 CreateLinearVelocity(Axis);
};

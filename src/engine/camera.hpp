#pragma once
#include "../common.hpp"
#include "../physics/BulletMain.h"

struct CameraProperties
{
    glm::vec3 origin = glm::vec3(0, 0, 0);
    float fov = glm::radians(60.0f);
    float aspectRatio = 4.0f / 3.0f;
    float nearClipPlane = 0.1f;
    float farClipPlane = 1000.0f;
};

class Camera 
{
private:
    glm::vec3 _origin;
    glm::vec2 _vhAngle;
    glm::mat4 _projection;
    glm::mat4 _view;
    btRigidBody* _rigidbody;

public:
    Camera(CameraProperties = CameraProperties(), glm::vec2 = glm::vec2(0, 0));

    void Embody(const std::shared_ptr<btDiscreteDynamicsWorld>&);

    void backAndForthMove(float);
    
    void damp();

    void horizontallyMove(float);

    void verticallyMove(float);

    void yaw(float);

    void pitch(float);

    glm::mat4 getDynamicsTransform();

    glm::mat4 getProjectionViewMatrix();

    bool isFreezing();

    glm::vec3 GetOrigin() 
    {
        glm::vec4 transformed_position_4d = getDynamicsTransform() * glm::vec4(_origin, 1.0);
        glm::vec3 transformed_position = glm::vec3(transformed_position_4d.x, transformed_position_4d.y, transformed_position_4d.z);
        return transformed_position;
    }

    void SetOrigin(glm::vec3 position) 
    {
        btTransform trans = btTransform(btQuaternion(0, 0, 0), btVector3(position.x, position.y, position.z));
        _rigidbody->proceedToTransform(trans);
    }
};

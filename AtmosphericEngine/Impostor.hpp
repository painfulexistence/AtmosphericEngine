#pragma once
#include "Globals.hpp" 
#include "BulletLinearMath.hpp"
#include "BulletCollision.hpp"
#include "BulletDynamics.hpp"
#include "PhysicsServer.hpp"
#include "GameObject.hpp"
#include "Component.hpp"

class Impostor : public Component
{
public:
    Impostor(GameObject* gameObject, btCollisionShape* shape, float mass = 0.0f);

    ~Impostor();

    std::string GetName() const override;

    glm::mat4 GetCenterOfMassWorldTransform();

    void SetCenterOfMassWorldTransform(const glm::mat4& transform);

    void Activate();

    void Dampen();

    // This will override the dynamics world gravity
    void SetGravity(const float& acc);

    glm::vec3 GetLinearFactor();

    void SetLinearFactor(const glm::vec3& fac);

    glm::vec3 GetLinearVelocity();

    void SetLinearVelocity(const glm::vec3& vel);

    glm::vec3 GetAngularFactor();

    void SetAngularFactor(const glm::vec3& fac);

    glm::vec3 GetAngularVelocity();

    void SetAngularVelocity(const glm::vec3& vel);

    void SetTransform(const glm::vec3& position, const glm::vec3& rotation);

    btRigidBody* Data() const;

private:
    btMotionState* _motionState;
    btRigidBody* _rigidbody;
    friend class PhysicsServer;
};
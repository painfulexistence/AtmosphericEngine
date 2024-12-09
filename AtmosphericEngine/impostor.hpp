#pragma once
#include "globals.hpp"
#include "bullet_linear_math.hpp"
#include "bullet_collision.hpp"
#include "bullet_dynamics.hpp"
#include "game_object.hpp"
#include "component.hpp"

class PhysicsServer;

class Impostor : public Component
{
public:
    Impostor(GameObject* gameObject, btCollisionShape* shape, float mass = 0.0f, glm::vec3 linearFactor = glm::vec3(1.0f), glm::vec3 angularFactor = glm::vec3(1.0f));
    ~Impostor();

    std::string GetName() const override;

    glm::mat4 GetWorldTransform();
    void SetWorldTransform(const glm::vec3& position, const glm::vec3& rotation);

    void Act();
    void Stop();
    void Dampen();

    // This will override the dynamics world gravity
    void SetGravity(const glm::vec3& acc);

    glm::vec3 GetLinearFactor();
    void SetLinearFactor(const glm::vec3& fac);
    glm::vec3 GetAngularFactor();
    void SetAngularFactor(const glm::vec3& fac);

    glm::vec3 GetLinearVelocity();
    void SetLinearVelocity(const glm::vec3& vel);
    glm::vec3 GetAngularVelocity();
    void SetAngularVelocity(const glm::vec3& vel);

    btRigidBody* Data() const;

private:
    btRigidBody* _rigidbody;
    friend class PhysicsServer;
};
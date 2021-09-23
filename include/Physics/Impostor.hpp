#pragma once
#include "Globals.hpp" 
#include "Physics/LinearMath.hpp"
#include "Physics/Collision.hpp"
#include "Physics/Dynamics.hpp"

static glm::mat4 ConvertToGLMatrix(const btTransform& trans)
{
    btScalar mat[16] = {0.0f};
    trans.getOpenGLMatrix(mat);
        
    return glm::mat4(
        mat[0], mat[1], mat[2], mat[3],
        mat[4], mat[5], mat[6], mat[7],
        mat[8], mat[9], mat[10], mat[11],
        mat[12], mat[13], mat[14], mat[15]
    );
}

struct ImpostorProps
{
    ImpostorProps(btCollisionShape* shape, const glm::vec3& position, const float& mass)
    {
        this->shape = shape;
        this->position = position;
        this->mass = mass;
    };
    btCollisionShape* shape;
    glm::vec3 position;
    float mass;
};

class Impostor
{
public:
    Impostor(const ImpostorProps& props)
    {
        glm::vec3 position = props.position;
        this->_motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0), btVector3(position.x, position.y, position.z)));
        this->_rigidbody = new btRigidBody(btScalar(props.mass), this->_motionState, props.shape, btVector3(1, 1, 1));
    };

    Impostor(btCollisionShape* shape, glm::vec3 position, float mass)
    {
        this->_motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0), btVector3(position.x, position.y, position.z)));
        this->_rigidbody = new btRigidBody(btScalar(mass), this->_motionState, shape, btVector3(1, 1, 1));
    };

    ~Impostor()
    {
        delete this->_rigidbody;
        delete this->_motionState;
    };

    glm::mat4 GetCenterOfMassWorldTransform()
    {
        btTransform t;
        this->_motionState->getWorldTransform(t);
        return ConvertToGLMatrix(t);
    };

    void SetCenterOfMassWorldTransform(const glm::mat4& transform)
    {
        // TODO:
    };

    void Activate()
    {
        this->_rigidbody->activate();
    }

    void Dampen()
    {
        btVector3 vel = this->_rigidbody->getLinearVelocity();
        this->_rigidbody->setLinearVelocity(btVector3(vel.x() / 2.0f, vel.y(), vel.z() / 2.0f));
    }

    // This will override the dynamics world gravity
    void SetGravity(const float& acc)
    {
        this->_rigidbody->setGravity(btVector3(0, -acc, 0));
    }

    glm::vec3 GetLinearFactor()
    {
        btVector3 fac = this->_rigidbody->getLinearFactor();
        return glm::vec3(fac.x(), fac.y(), fac.z());
    }

    void SetLinearFactor(const glm::vec3& fac)
    {
        this->_rigidbody->setLinearFactor(btVector3(fac.x, fac.y, fac.z));
    }

    glm::vec3 GetLinearVelocity()
    {
        btVector3 vel = this->_rigidbody->getLinearVelocity();
        return glm::vec3(vel.x(), vel.y(), vel.z());
    }

    void SetLinearVelocity(const glm::vec3& vel)
    {
        this->_rigidbody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
    }

    glm::vec3 GetAngularFactor()
    {
        btVector3 fac = this->_rigidbody->getAngularFactor();
        return glm::vec3(fac.x(), fac.y(), fac.z());
    }

    void SetAngularFactor(const glm::vec3& fac)
    {
        this->_rigidbody->setAngularFactor(btVector3(fac.x, fac.y, fac.z));
    }

    glm::vec3 GetAngularVelocity()
    {
        btVector3 vel = this->_rigidbody->getAngularVelocity();
        return glm::vec3(vel.x(), vel.y(), vel.z());
    }

    void SetAngularVelocity(const glm::vec3& vel)
    {
        this->_rigidbody->setAngularVelocity(btVector3(vel.x, vel.y, vel.z));
    }

    void SetTransform(const glm::vec3& position, const glm::vec3& rotation)
    {
        btTransform t = btTransform(btQuaternion(rotation.x, rotation.y, rotation.z), btVector3(position.x, position.y, position.z));
        this->_rigidbody->proceedToTransform(t);
    }

    btRigidBody* Data() const
    {
        return this->_rigidbody;
    }

private:
    btMotionState* _motionState;
    btRigidBody* _rigidbody;
};
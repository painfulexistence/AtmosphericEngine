#include "Impostor.hpp"
#include "GameObject.hpp"

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

Impostor::Impostor(GameObject* gameObject, btCollisionShape* shape, float mass)
{
    glm::vec3 position = gameObject->GetPosition();
    glm::vec3 rotation = gameObject->GetRotation();
    btTransform transform = btTransform(btQuaternion(rotation.x, rotation.y, rotation.z), btVector3(position.x, position.y, position.z));
    _motionState = new btDefaultMotionState(transform);
    _rigidbody = new btRigidBody(btScalar(mass), this->_motionState, shape, btVector3(1, 1, 1));    
    
    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
};

Impostor::~Impostor()
{
    // TODO: Check if bullet objects are destoryed by the destructor in btDynamicsWorld class
    //delete this->_rigidbody;
    //delete this->_motionState;
};

std::string Impostor::GetName() const
{
    return std::string("Physics");
}

glm::mat4 Impostor::GetCenterOfMassWorldTransform()
{
    btTransform t;
    this->_motionState->getWorldTransform(t); //getMotionState()->getWorldTransform(t)
    return ConvertToGLMatrix(t);
};

void Impostor::SetCenterOfMassWorldTransform(const glm::mat4& transform)
{
    // TODO:
};

void Impostor::Activate()
{
    this->_rigidbody->activate();
}

void Impostor::Dampen()
{
    btVector3 vel = this->_rigidbody->getLinearVelocity();
    this->_rigidbody->setLinearVelocity(btVector3(vel.x() / 2.0f, vel.y(), vel.z() / 2.0f));
}

// This will override the dynamics world gravity
void Impostor::SetGravity(const float& acc)
{
    this->_rigidbody->setGravity(btVector3(0, -acc, 0));
}

glm::vec3 Impostor::GetLinearFactor()
{
    btVector3 fac = this->_rigidbody->getLinearFactor();
    return glm::vec3(fac.x(), fac.y(), fac.z());
}

void Impostor::SetLinearFactor(const glm::vec3& fac)
{
    this->_rigidbody->setLinearFactor(btVector3(fac.x, fac.y, fac.z));
}

glm::vec3 Impostor::GetLinearVelocity()
{
    btVector3 vel = this->_rigidbody->getLinearVelocity();
    return glm::vec3(vel.x(), vel.y(), vel.z());
}

void Impostor::SetLinearVelocity(const glm::vec3& vel)
{
    this->_rigidbody->activate();
    this->_rigidbody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
}

glm::vec3 Impostor::GetAngularFactor()
{
    btVector3 fac = this->_rigidbody->getAngularFactor();
    return glm::vec3(fac.x(), fac.y(), fac.z());
}

void Impostor::SetAngularFactor(const glm::vec3& fac)
{
    this->_rigidbody->setAngularFactor(btVector3(fac.x, fac.y, fac.z));
}

glm::vec3 Impostor::GetAngularVelocity()
{
    btVector3 vel = this->_rigidbody->getAngularVelocity();
    return glm::vec3(vel.x(), vel.y(), vel.z());
}

void Impostor::SetAngularVelocity(const glm::vec3& vel)
{
    this->_rigidbody->activate();
    this->_rigidbody->setAngularVelocity(btVector3(vel.x, vel.y, vel.z));
}

void Impostor::SetTransform(const glm::vec3& position, const glm::vec3& rotation)
{
    btTransform t = btTransform(btQuaternion(rotation.x, rotation.y, rotation.z), btVector3(position.x, position.y, position.z));
    this->_rigidbody->proceedToTransform(t);
}

btRigidBody* Impostor::Data() const
{
    return this->_rigidbody;
}
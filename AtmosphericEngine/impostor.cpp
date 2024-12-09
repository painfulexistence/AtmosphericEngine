#include "impostor.hpp"
#include "game_object.hpp"

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

Impostor::Impostor(GameObject* gameObject, btCollisionShape* shape, float mass, glm::vec3 linearFactor, glm::vec3 angularFactor)
{
    glm::vec3 position = gameObject->GetPosition();
    glm::vec3 rotation = gameObject->GetRotation();

    btTransform t;
    t.setIdentity();
    t.setOrigin(btVector3(position.x, position.y, position.z));
    t.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, 1.0f));

    auto motionState = new btDefaultMotionState(t);
    _rigidbody = new btRigidBody(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    _rigidbody->setLinearFactor(btVector3(linearFactor.x, linearFactor.y, linearFactor.z));
    _rigidbody->setAngularFactor(btVector3(angularFactor.x, angularFactor.y, angularFactor.z));
    _rigidbody->setDamping(0.9f, 0.0f);
    // _rigidbody->setCollisionFlags(_rigidbody->getCollisionFlags() |
    //     btCollisionObject::CF_NO_CONTACT_RESPONSE);
    _rigidbody->setUserPointer(gameObject);

    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
};

Impostor::~Impostor()
{
    // TODO: Check if bullet objects are destoryed by the destructor in btDynamicsWorld class
    //delete this->_rigidbody;
};

std::string Impostor::GetName() const
{
    return std::string("Physics");
}

glm::mat4 Impostor::GetWorldTransform()
{
    btTransform t;
    _rigidbody->getMotionState()->getWorldTransform(t); //getMotionState()->getWorldTransform(t)
    return ConvertToGLMatrix(t);
};

void Impostor::SetWorldTransform(const glm::vec3& position, const glm::vec3& rotation)
{
    btTransform t;
    t.setIdentity();
    t.setOrigin(btVector3(position.x, position.y, position.z));
    t.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, 1.0f));
    _rigidbody->setWorldTransform(t);
    _rigidbody->getMotionState()->setWorldTransform(t);
}

void Impostor::Act()
{
    _rigidbody->activate();
}

void Impostor::Stop()
{
    _rigidbody->setActivationState(0);
}

void Impostor::Dampen()
{
    btVector3 vel = _rigidbody->getLinearVelocity();
    _rigidbody->setLinearVelocity(btVector3(vel.x() / 2.0f, vel.y(), vel.z() / 2.0f));
}

// This will override the dynamics world gravity
void Impostor::SetGravity(const glm::vec3& acc)
{
    _rigidbody->setGravity(btVector3(acc.x, acc.y, acc.z));
}

glm::vec3 Impostor::GetLinearFactor()
{
    btVector3 fac = _rigidbody->getLinearFactor();
    return glm::vec3(fac.x(), fac.y(), fac.z());
}

void Impostor::SetLinearFactor(const glm::vec3& fac)
{
    _rigidbody->setLinearFactor(btVector3(fac.x, fac.y, fac.z));
}

glm::vec3 Impostor::GetLinearVelocity()
{
    btVector3 vel = _rigidbody->getLinearVelocity();
    return glm::vec3(vel.x(), vel.y(), vel.z());
}

void Impostor::SetLinearVelocity(const glm::vec3& vel)
{
    _rigidbody->activate();
    _rigidbody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
}

glm::vec3 Impostor::GetAngularFactor()
{
    btVector3 fac = _rigidbody->getAngularFactor();
    return glm::vec3(fac.x(), fac.y(), fac.z());
}

void Impostor::SetAngularFactor(const glm::vec3& fac)
{
    _rigidbody->setAngularFactor(btVector3(fac.x, fac.y, fac.z));
}

glm::vec3 Impostor::GetAngularVelocity()
{
    btVector3 vel = _rigidbody->getAngularVelocity();
    return glm::vec3(vel.x(), vel.y(), vel.z());
}

void Impostor::SetAngularVelocity(const glm::vec3& vel)
{
    _rigidbody->activate();
    _rigidbody->setAngularVelocity(btVector3(vel.x, vel.y, vel.z));
}



btRigidBody* Impostor::Data() const
{
    return _rigidbody;
}
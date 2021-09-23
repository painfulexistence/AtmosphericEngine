#include "Physics/DynamicsWorld.hpp"

DynamicsWorld::DynamicsWorld()
{
    this->_config = new btDefaultCollisionConfiguration();
    this->_dispatcher = new btCollisionDispatcher(_config);
    this->_broadphase = new btDbvtBroadphase();
    this->_solver = new btSequentialImpulseConstraintSolver();
    this->_dw = new btDiscreteDynamicsWorld(_dispatcher, _broadphase, _solver, _config);
    this->_timeAccum = 0.0f;
}

DynamicsWorld::~DynamicsWorld()
{
    delete this->_dw;
    delete this->_solver;
    delete this->_broadphase;
    delete this->_dispatcher;
    delete this->_config;
}

void DynamicsWorld::SetConstantGravity(const float& g)
{ 
    _dw->setGravity(btVector3(0, -g, 0)); 
}

void DynamicsWorld::Step(float dt)
{
    _timeAccum += dt;
    while (_timeAccum >= FIXED_TIME_STEP) 
    {
        _dw->stepSimulation(FIXED_TIME_STEP, 0);
        _timeAccum -= FIXED_TIME_STEP;
    }
}

btTransform DynamicsWorld::GetImpostorTransform(uint64_t id) const
{
    if (_impostors.count(id) == 0)
        throw std::runtime_error("Cannot find the impostor!");

    btTransform trans;
    _impostors.find(id)->second->getMotionState()->getWorldTransform(trans);
    return trans;
}

uint64_t DynamicsWorld::CreateImpostor(btCollisionShape* shape, btVector3 position, float mass)
{
    btTransform trans = btTransform(btQuaternion(0, 0, 0), position);
    btMotionState* motionState = new btDefaultMotionState(trans);
    //btCollisionShape* shape = new btCapsuleShape(btScalar(diameter / 2.0f), btScalar(height / 2.0f));
    btRigidBody* rigid = new btRigidBody(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    _dw->addRigidBody(rigid);

    uint64_t eid = _impostors.size() + 1;
    _impostors.insert({eid, rigid});

    return eid;
}

bool DynamicsWorld::GetImpostorLinearFactor(uint64_t id, btVector3& fac) const
{
    if (!HasImpostor(id)) 
        return false;
    
    fac = _impostors.find(id)->second->getLinearFactor();
    return true;
}

void DynamicsWorld::SetImpostorLinearFactor(uint64_t id, const btVector3& fac)
{
    if (!HasImpostor(id)) 
        return;

    _impostors.find(id)->second->setLinearFactor(fac);
}

bool DynamicsWorld::GetImpostorLinearVelocity(uint64_t id, btVector3& vel) const
{
    if (!HasImpostor(id)) 
        return false;

    vel = _impostors.find(id)->second->getLinearVelocity();
    return true;
}

void DynamicsWorld::SetImpostorLinearVelocity(uint64_t id, const btVector3& vel)
{
    if (!HasImpostor(id)) 
        return;

    btRigidBody*& r = _impostors.find(id)->second;
    r->activate();
    r->setLinearVelocity(vel);
}

bool DynamicsWorld::GetImpostorAngularFactor(uint64_t id, btVector3& fac) const
{
    if (!HasImpostor(id)) 
        return false;
    
    fac = _impostors.find(id)->second->getAngularFactor();
    return true;
}

void DynamicsWorld::SetImpostorAngularFactor(uint64_t id, const btVector3& fac)
{
    if (!HasImpostor(id)) 
        return;

    _impostors.find(id)->second->setAngularFactor(fac);
}

bool DynamicsWorld::GetImpostorAngularVelocity(uint64_t id, btVector3& vel) const
{
    if (!HasImpostor(id)) 
        return false;

    vel = _impostors.find(id)->second->getAngularVelocity();
    return true;
}

void DynamicsWorld::SetImpostorAngularVelocity(uint64_t id, const btVector3& vel)
{
    if (!HasImpostor(id)) 
        return;

    btRigidBody*& r = _impostors.find(id)->second;
    r->activate();
    r->setAngularVelocity(vel);
}

void DynamicsWorld::ActivateImpostor(uint64_t id)
{
    if (!HasImpostor(id)) 
        return;

    _impostors.find(id)->second->activate();
}

void DynamicsWorld::DampenImpostor(uint64_t id)
{
    if (!HasImpostor(id)) 
        return;

    btRigidBody*& r = _impostors.find(id)->second;
    btVector3 vel = r->getLinearVelocity();
    r->setLinearVelocity(btVector3(vel.x() / 2.0f, vel.y(), vel.z() / 2.0f));
}

void DynamicsWorld::RotateImpostor(uint64_t id, btQuaternion rotation) // Rotate to
{
    if (!HasImpostor(id))
        return;

    _impostors.find(id)->second->proceedToTransform(btTransform(rotation, btVector3(0, 0, 0)));
}

void DynamicsWorld::TranslateImpostor(uint64_t id, btVector3 translation) // Translate to
{
    if (!HasImpostor(id))
        return;

    _impostors.find(id)->second->proceedToTransform(btTransform(btQuaternion(0, 0, 0), translation));
}

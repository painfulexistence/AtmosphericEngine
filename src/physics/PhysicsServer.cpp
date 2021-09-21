#include "Physics/PhysicsServer.hpp"

PhysicsServer::PhysicsServer() : _timeAccum(0.0f)
{
    _config = new btDefaultCollisionConfiguration();
    _dispatcher = new btCollisionDispatcher(_config);
    _broadphase = new btDbvtBroadphase();
    _solver = new btSequentialImpulseConstraintSolver();
    _world = new btDiscreteDynamicsWorld(_dispatcher, _broadphase, _solver, _config);
    _world->setGravity(btVector3(0, -GRAVITY, 0));
}

PhysicsServer::~PhysicsServer()
{
    delete _world;
    delete _solver;
    delete _broadphase;
    delete _dispatcher;
    delete _config;
}

void PhysicsServer::Process(float dt)
{    
    _timeAccum += dt;
    while (_timeAccum >= FIXED_TIME_STEP) {
        _world->stepSimulation(FIXED_TIME_STEP, 0);
        _timeAccum -= FIXED_TIME_STEP;
    }
}

std::uint64_t PhysicsServer::CreateBoxImpostor(btVector3 position, btVector3 size, float mass)
{
    btTransform trans = btTransform(btQuaternion(0, 0, 0), position);
    btMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btBoxShape(size / 2.0f);
    btRigidBody* rigid = new btRigidBody(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    _world->addRigidBody(rigid);

    std::uint64_t eid = _impostors.size() + 1;
    _impostors.insert({eid, rigid});

    return eid;
}

std::uint64_t PhysicsServer::CreateSphereImpostor(btVector3 position, float diameter, float mass)
{
    btTransform trans = btTransform(btQuaternion(0, 0, 0), position);    
    btMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btSphereShape(btScalar(diameter / 2.0f));
    btRigidBody* rigid = new btRigidBody(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    _world->addRigidBody(rigid);

    std::uint64_t eid = _impostors.size() + 1;
    _impostors.insert({eid, rigid});

    return eid;
}

std::uint64_t PhysicsServer::CreateCapsuleImpostor(btVector3 position, float diameter, float height, float mass)
{
    btTransform trans = btTransform(btQuaternion(0, 0, 0), position);
    btMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btCapsuleShape(btScalar(diameter / 2.0f), btScalar(height / 2.0f));
    btRigidBody* rigid = new btRigidBody(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    _world->addRigidBody(rigid);

    std::uint64_t eid = _impostors.size() + 1;
    _impostors.insert({eid, rigid});

    return eid;
}

bool PhysicsServer::GetImpostorLinearFactor(std::uint64_t id, btVector3& fac) const
{
    if (!HasImpostor(id)) 
        return false;
    
    fac = _impostors.find(id)->second->getLinearFactor();
    return true;
}

void PhysicsServer::SetImpostorLinearFactor(std::uint64_t id, const btVector3& fac)
{
    if (!HasImpostor(id)) 
        return;

    _impostors.find(id)->second->setLinearFactor(fac);
}

bool PhysicsServer::GetImpostorLinearVelocity(std::uint64_t id, btVector3& vel) const
{
    if (!HasImpostor(id)) 
        return false;

    vel = _impostors.find(id)->second->getLinearVelocity();
    return true;
}

void PhysicsServer::SetImpostorLinearVelocity(std::uint64_t id, const btVector3& vel)
{
    if (!HasImpostor(id)) 
        return;

    btRigidBody*& r = _impostors.find(id)->second;
    r->activate();
    r->setLinearVelocity(vel);
}

bool PhysicsServer::GetImpostorAngularFactor(std::uint64_t id, btVector3& fac) const
{
    if (!HasImpostor(id)) 
        return false;
    
    fac = _impostors.find(id)->second->getAngularFactor();
    return true;
}

void PhysicsServer::SetImpostorAngularFactor(std::uint64_t id, const btVector3& fac)
{
    if (!HasImpostor(id)) 
        return;

    _impostors.find(id)->second->setAngularFactor(fac);
}

bool PhysicsServer::GetImpostorAngularVelocity(std::uint64_t id, btVector3& vel) const
{
    if (!HasImpostor(id)) 
        return false;

    vel = _impostors.find(id)->second->getAngularVelocity();
    return true;
}

void PhysicsServer::SetImpostorAngularVelocity(std::uint64_t id, const btVector3& vel)
{
    if (!HasImpostor(id)) 
        return;

    btRigidBody*& r = _impostors.find(id)->second;
    r->activate();
    r->setAngularVelocity(vel);
}

void PhysicsServer::ActivateImpostor(std::uint64_t id)
{
    if (!HasImpostor(id)) 
        return;

    _impostors.find(id)->second->activate();
}

void PhysicsServer::DampenImpostor(std::uint64_t id)
{
    if (!HasImpostor(id)) 
        return;

    btRigidBody*& r = _impostors.find(id)->second;
    btVector3 vel = r->getLinearVelocity();
    r->setLinearVelocity(btVector3(vel.x() / 2.0f, vel.y(), vel.z() / 2.0f));
}

void PhysicsServer::RotateImpostor(std::uint64_t id, btQuaternion rotation) // Rotate to
{
    if (!HasImpostor(id))
        return;

    _impostors.find(id)->second->proceedToTransform(btTransform(rotation, btVector3(0, 0, 0)));
}

void PhysicsServer::TranslateImpostor(std::uint64_t id, btVector3 translation) // Translate to
{
    if (!HasImpostor(id))
        return;

    _impostors.find(id)->second->proceedToTransform(btTransform(btQuaternion(0, 0, 0), translation));
}

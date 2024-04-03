#include "PhysicsServer.hpp"
#include "Impostor.hpp"

PhysicsServer::PhysicsServer()
{
    _world = new DynamicsWorld();
    _world->SetConstantGravity(GRAVITY);
}

PhysicsServer::~PhysicsServer()
{
    delete this->_world;
}

void PhysicsServer::Process(float dt)
{
    _world->Step(dt);
}

DynamicsWorld* PhysicsServer::World()
{
    return this->_world;
}

void PhysicsServer::AddImpostor(Impostor* impostor)
{
    _world->AddRigidbody(impostor->_rigidbody);
}
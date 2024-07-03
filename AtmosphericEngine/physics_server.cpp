#include "physics_server.hpp"
#include "physics_world.hpp"
#include "impostor.hpp"

PhysicsServer::PhysicsServer()
{

}

PhysicsServer::~PhysicsServer()
{

}

void PhysicsServer::Init(Application* app)
{
    Server::Init(app);

    _world = std::make_shared<PhysicsWorld>();
    _world->SetConstantGravity(GRAVITY);
}

void PhysicsServer::Process(float dt)
{
    _world->Step(dt);
}

void PhysicsServer::AddImpostor(Impostor* impostor)
{
    _world->AddRigidbody(impostor->_rigidbody);
}

void PhysicsServer::RemoveImpostor(Impostor* impostor)
{
    _world->RemoveRigidbody(impostor->_rigidbody);
}

void PhysicsServer::RenderDebug()
{
    _world->RenderDebug();
}
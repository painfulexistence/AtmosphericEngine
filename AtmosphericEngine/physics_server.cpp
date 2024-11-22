#include "physics_server.hpp"
#include "physics_world.hpp"
#include "impostor.hpp"

PhysicsServer* PhysicsServer::_instance = nullptr;

PhysicsServer::PhysicsServer()
{
    if (_instance != nullptr)
        throw std::runtime_error("Physics server is already initialized!");

    _instance = this;
}

PhysicsServer::~PhysicsServer()
{

}

void PhysicsServer::Init(Application* app)
{
    Server::Init(app);

    _world = std::make_unique<PhysicsWorld>();
    _world->SetGravity(GRAVITY);
}

void PhysicsServer::Process(float dt)
{
    _world->Update(dt);
    if (_debugUIEnabled)
    {
        _world->DrawDebug(); // TODO: check if this cost performance when debug mode is NoDebug
    }
}

void PhysicsServer::AddImpostor(Impostor* impostor)
{
    _world->AddRigidbody(impostor->_rigidbody);
}

void PhysicsServer::RemoveImpostor(Impostor* impostor)
{
    _world->RemoveRigidbody(impostor->_rigidbody);
}

void PhysicsServer::EnableDebugUI(bool enable)
{
    _debugUIEnabled = enable;
}
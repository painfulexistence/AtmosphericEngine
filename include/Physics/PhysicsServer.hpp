#pragma once
#include "Globals.hpp"
#include "Framework.hpp"
#include "Physics/DynamicsWorld.hpp"

class PhysicsServer : public Server
{
private:
    DynamicsWorld* _world;
    Application* _app;
    std::map<uint64_t, btRigidBody*> _impostors;

public:
    PhysicsServer();

    ~PhysicsServer();

    void Process(float dt) override;

    DynamicsWorld* World() { return this->_world; }
};
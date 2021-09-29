#pragma once
#include "Globals.hpp"
#include "Framework.hpp"
#include "Physics/DynamicsWorld.hpp"

class Impostor;

class PhysicsServer : public Server
{
private:
    DynamicsWorld* _world;
    Application* _app;
    std::vector<Impostor*> _impostors;

public:
    PhysicsServer();

    ~PhysicsServer();

    void AddImpostor(Impostor*);

    void Process(float dt) override;

    DynamicsWorld* World();
};
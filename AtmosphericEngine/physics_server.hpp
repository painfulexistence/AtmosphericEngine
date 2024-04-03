#pragma once
#include "Globals.hpp"
#include "Server.hpp"
#include "DynamicsWorld.hpp"
#include "Impostor.hpp"

class PhysicsServer : public Server
{
private:
    DynamicsWorld* _world;
    std::vector<Impostor*> _impostors;

public:
    PhysicsServer();

    ~PhysicsServer();

    void AddImpostor(Impostor*);

    void Process(float dt) override;

    DynamicsWorld* World();
};
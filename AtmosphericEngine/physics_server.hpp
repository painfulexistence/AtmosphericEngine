#pragma once
#include "globals.hpp"
#include "server.hpp"
#include "dynamics_world.hpp"
#include "impostor.hpp"

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
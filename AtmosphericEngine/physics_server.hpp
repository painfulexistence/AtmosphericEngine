#pragma once
#include "globals.hpp"
#include "server.hpp"
#include <memory>

class Impostor;

class PhysicsWorld;

class PhysicsServer : public Server
{
private:
    std::shared_ptr<PhysicsWorld> _world;
    std::vector<Impostor*> _impostors;

public:
    PhysicsServer();

    ~PhysicsServer();

    void Init(Application* app);

    virtual void Process(float dt) override;

    void AddImpostor(Impostor*);

    void RemoveImpostor(Impostor*);

    void RenderDebug();
};
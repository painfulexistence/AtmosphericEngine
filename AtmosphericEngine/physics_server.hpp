#pragma once
#include "globals.hpp"
#include "server.hpp"
#include <memory>

class Impostor;

class PhysicsWorld;

class PhysicsServer : public Server
{
private:
    static PhysicsServer* _instance;

    std::shared_ptr<PhysicsWorld> _world;
    std::vector<Impostor*> _impostors;
    bool _debugUIEnabled = false;

public:
    static PhysicsServer* Get()
    {
        return _instance;
    }

    PhysicsServer();

    ~PhysicsServer();

    void Init(Application* app) override;

    void Process(float dt) override;

    void AddImpostor(Impostor*);

    void RemoveImpostor(Impostor*);

    void EnableDebugUI(bool enable = true);
};
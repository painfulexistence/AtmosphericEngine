#pragma once
#include "Globals.hpp"
#include "Framework.hpp"

struct Transform;

struct Geometry;

struct Impostor;

struct Script;

class EnttRegistry;

class ECSServer : public Server
{
public:
    ECSServer();

    ~ECSServer();

    void Process(float dt) override;

    void OnMessage(Message msg) override;

    uint64_t Create();

    void Destroy(uint64_t eid);

    void SyncWithPhysics();

    void AddGraphicsComponent(uint64_t eid)
    {

    }

    void RemoveGraphicsComponent(uint64_t eid)
    {

    }

    void AddPhysicsComponent(uint64_t eid)
    {

    }

    void RemovePhysicsComponent(uint64_t eid)
    {

    }

    void AddScriptComponent(uint64_t eid)
    {

    }

    void RemoveScriptComponent(uint64_t eid)
    {

    }

private:
    EnttRegistry* _registry;
};
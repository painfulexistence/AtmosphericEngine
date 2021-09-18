#pragma once
#include "Globals.hpp"

class Entity
{
protected:
    std::uint64_t _id = 0;
    std::uint64_t _graphicsId = 0;
    std::uint64_t _physicsId = 0;

public:
    static std::list<Entity> Entities;

    static std::list<Entity> WithGraphicsComponent();

    static std::list<Entity> WithPhysicsComponent();

    Entity();
    
    std::uint64_t GetGraphicsId() const { return _graphicsId; }

    void SetGraphicsId(std::uint64_t gid) { _graphicsId = gid; }

    std::uint64_t GetPhysicsId() const { return _physicsId; }

    void SetPhysicsId(std::uint64_t pid) { _physicsId = pid; }
};
#pragma once
#include "Globals.hpp"

class Entity
{
public:
    static std::list<Entity> Entities;

    static std::list<Entity> WithGraphicsComponent();

    static std::list<Entity> WithPhysicsComponent();
    
    Entity();

    std::uint64_t GetEID() const { return _eid; }
    
    std::uint64_t GetGraphicsId() const { return _graphicsId; }

    void SetGraphicsId(const std::uint64_t& gid) { _graphicsId = gid; }

    std::uint64_t GetPhysicsId() const { return _physicsId; }

    void SetPhysicsId(const std::uint64_t& pid) { _physicsId = pid; }

    inline bool operator==(const Entity& rhs) { return (GetEID() == rhs.GetEID()); };

protected:
    std::uint64_t _eid = 0;
    std::uint64_t _graphicsId = 0;
    std::uint64_t _physicsId = 0;
};
#pragma once
#include "BulletMinimal.h"
#include <bullet/btBulletDynamicsCommon.h> // This header contains all bullet headers needed
#include <map>
#include <utility>

class PhysicsWorld
{
private:
    btCollisionConfiguration* _config;
    btCollisionDispatcher* _dispatcher;
    btBroadphaseInterface* _broadphase;
    btConstraintSolver* _solver;
    btDiscreteDynamicsWorld* _world;
    float _timeAccum;
    std::map<std::uint64_t, btRigidBody*> _impostors;

public:
    PhysicsWorld();

    ~PhysicsWorld();

    void SetGravity(float gravity) { _world->setGravity(btVector3(0, -gravity, 0)); }

    bool HasImpostor(std::uint64_t id) const { return _impostors.count(id) != 0; }

    btTransform GetImpostorTransform(std::uint64_t id) const
    {
        if (_impostors.count(id) == 0)
            throw std::runtime_error("Cannot find the impostor!");

        btTransform trans;
        _impostors.find(id)->second->getMotionState()->getWorldTransform(trans);
        return trans;
    }
    
    std::uint64_t CreateBoxImpostor(btVector3 position, btVector3 size = btVector3(1.f, 1.f, 1.f), float mass = 0.f);

    std::uint64_t CreateSphereImpostor(btVector3 position, float diameter = 1.f, float mass = 0.f);

    std::uint64_t CreateCapsuleImpostor(btVector3 position, float diameter = 1.f, float height = 1.f, float mass = 0.f);

    bool GetImpostorLinearFactor(std::uint64_t, btVector3&) const;

    void SetImpostorLinearFactor(std::uint64_t, const btVector3&);

    bool GetImpostorLinearVelocity(std::uint64_t, btVector3&) const;

    void SetImpostorLinearVelocity(std::uint64_t, const btVector3&);

    bool GetImpostorAngularFactor(std::uint64_t, btVector3&) const;

    void SetImpostorAngularFactor(std::uint64_t, const btVector3&);

    bool GetImpostorAngularVelocity(std::uint64_t, btVector3&) const;

    void SetImpostorAngularVelocity(std::uint64_t, const btVector3&);

    void ActivateImpostor(std::uint64_t);

    void DampenImpostor(std::uint64_t);

    void RotateImpostor(std::uint64_t, btQuaternion);

    void TranslateImpostor(std::uint64_t, btVector3);

    void Update(float dt);
};
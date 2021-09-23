#pragma once
#include "Globals.hpp"
#include "Physics/Dynamics.hpp"
#include "Physics/Collision.hpp"
#include "Physics/Impostor.hpp"

class DynamicsWorld
{
public:
    DynamicsWorld();

    ~DynamicsWorld();

    void SetConstantGravity(const float& g);

    void Step(float dt);

    bool HasImpostor(uint64_t id) const { return _impostors.count(id) != 0; }

    btTransform GetImpostorTransform(uint64_t id) const;
    
    uint64_t CreateImpostor(btCollisionShape* shape, btVector3 position, float mass = 0.f);

    bool GetImpostorLinearFactor(uint64_t, btVector3&) const;

    void SetImpostorLinearFactor(uint64_t, const btVector3&);

    bool GetImpostorLinearVelocity(uint64_t, btVector3&) const;

    void SetImpostorLinearVelocity(uint64_t, const btVector3&);

    bool GetImpostorAngularFactor(uint64_t, btVector3&) const;

    void SetImpostorAngularFactor(uint64_t, const btVector3&);

    bool GetImpostorAngularVelocity(uint64_t, btVector3&) const;

    void SetImpostorAngularVelocity(uint64_t, const btVector3&);

    void ActivateImpostor(uint64_t);

    void DampenImpostor(uint64_t);

    void RotateImpostor(uint64_t, btQuaternion);

    void TranslateImpostor(uint64_t, btVector3);

private:
    btCollisionConfiguration* _config;
    btCollisionDispatcher* _dispatcher;
    btBroadphaseInterface* _broadphase;
    btConstraintSolver* _solver;
    btDiscreteDynamicsWorld* _dw;
    float _timeAccum;
    std::map<uint64_t, btRigidBody*> _impostors;
};
#pragma once
#include "Globals.hpp"
#include "Physics/BulletDynamics.hpp"
#include "Physics/BulletCollision.hpp"

class DynamicsWorld
{
public:
    DynamicsWorld();

    ~DynamicsWorld();

    void AddRigidbody(btRigidBody* rb);

    void SetConstantGravity(const float& g);

    void Step(float dt);

private:
    btCollisionConfiguration* _config;
    btCollisionDispatcher* _dispatcher;
    btBroadphaseInterface* _broadphase;
    btConstraintSolver* _solver;
    btDiscreteDynamicsWorld* _dw;
    float _timeAccum;
};
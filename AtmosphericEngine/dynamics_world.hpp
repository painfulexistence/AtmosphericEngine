#pragma once
#include "globals.hpp"
#include "bullet_dynamics.hpp"
#include "bullet_collision.hpp"

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
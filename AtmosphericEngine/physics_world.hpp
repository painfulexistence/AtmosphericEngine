#pragma once
#include "globals.hpp"
#include "bullet_dynamics.hpp"
#include "bullet_collision.hpp"
#include "physics_debug_drawer.hpp"

class PhysicsWorld
{
public:
    PhysicsWorld();

    ~PhysicsWorld();

    void AddRigidbody(btRigidBody* rb);

    void RemoveRigidbody(btRigidBody* rb);

    void SetConstantGravity(const float& g);

    void Step(float dt);

    void RenderDebug();

private:
    btCollisionConfiguration* _config;
    btCollisionDispatcher* _dispatcher;
    btBroadphaseInterface* _broadphase;
    btConstraintSolver* _solver;
    btDiscreteDynamicsWorld* _world;
    DebugDrawer* _debugDrawer;

    float _timeAccum;
};
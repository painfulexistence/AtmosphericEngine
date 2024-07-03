#pragma once
#include "globals.hpp"
#include "bullet_dynamics.hpp"
#include "bullet_collision.hpp"

class PhysicsDebugDrawer;

class PhysicsWorld
{
public:
    PhysicsWorld();

    ~PhysicsWorld();

    void AddRigidbody(btRigidBody* rb);

    void RemoveRigidbody(btRigidBody* rb);

    void SetGravity(const float& g);

    void SetGravity(const glm::vec3& g);

    void Update(float dt);

    void DrawDebug();

private:
    btCollisionConfiguration* _config;
    btCollisionDispatcher* _dispatcher;
    btBroadphaseInterface* _broadphase;
    btConstraintSolver* _solver;
    btDiscreteDynamicsWorld* _world;
    PhysicsDebugDrawer* _debugDrawer;

    float _timeAccum;
};
#include "physics_world.hpp"

PhysicsWorld::PhysicsWorld()
{
    _config = new btDefaultCollisionConfiguration();
    _dispatcher = new btCollisionDispatcher(_config);
    _broadphase = new btDbvtBroadphase();
    _solver = new btSequentialImpulseConstraintSolver();
    _world = new btDiscreteDynamicsWorld(_dispatcher, _broadphase, _solver, _config);

    _debugDrawer = new DebugDrawer();
    _world->setDebugDrawer(_debugDrawer);

    _timeAccum = 0.0f;
}

PhysicsWorld::~PhysicsWorld()
{
    delete _world;
    delete _debugDrawer;

    delete _solver;
    delete _broadphase;
    delete _dispatcher;
    delete _config;
}

void PhysicsWorld::AddRigidbody(btRigidBody* rb)
{
    _world->addRigidBody(rb);
}

void PhysicsWorld::RemoveRigidbody(btRigidBody* rb)
{
    _world->removeRigidBody(rb);
}

void PhysicsWorld::SetConstantGravity(const float& g)
{
    _world->setGravity(btVector3(0, -g, 0));
}

void PhysicsWorld::Step(float dt)
{
    _timeAccum += dt;
    while (_timeAccum >= FIXED_TIME_STEP)
    {
        _world->stepSimulation(FIXED_TIME_STEP, 0);
        _timeAccum -= FIXED_TIME_STEP;
    }
}

void PhysicsWorld::RenderDebug()
{
    _world->debugDrawWorld();
    _debugDrawer->Render();
}


#include "physics_world.hpp"
#include "physics_debug_drawer.hpp"
#include "game_object.hpp"

PhysicsWorld::PhysicsWorld()
{
    _config = new btDefaultCollisionConfiguration();
    _dispatcher = new btCollisionDispatcher(_config);
    _broadphase = new btDbvtBroadphase();
    _solver = new btSequentialImpulseConstraintSolver();
    _world = new btDiscreteDynamicsWorld(_dispatcher, _broadphase, _solver, _config);

    _debugDrawer = new PhysicsDebugDrawer();
    _world->setDebugDrawer(_debugDrawer);
    // 0: no debug
    // 1: wireframe
    // 14: fast wireframe
    _debugDrawer->setDebugMode(1);

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

void PhysicsWorld::SetGravity(const float& g)
{
    _world->setGravity(btVector3(0, -g, 0));
}

void PhysicsWorld::SetGravity(const glm::vec3& g)
{
    _world->setGravity(btVector3(g.x, g.y, g.z));
}

void PhysicsWorld::Update(float dt)
{
    _timeAccum += dt;
    while (_timeAccum >= FIXED_TIME_STEP)
    {
        _world->stepSimulation(FIXED_TIME_STEP, 0);
        _timeAccum -= FIXED_TIME_STEP;
    }

    int numManifolds = _dispatcher->getNumManifolds();
    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold* contactManifold = _dispatcher->getManifoldByIndexInternal(i);

        btCollisionObject* objA =
            const_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* objB =
            const_cast<btCollisionObject*>(contactManifold->getBody1());

        GameObject* gameObjA = static_cast<GameObject*>(objA->getUserPointer());
        GameObject* gameObjB = static_cast<GameObject*>(objB->getUserPointer());

        if (gameObjA && gameObjB) {
            if (contactManifold->getNumContacts() > 0) {
                gameObjA->OnCollision(gameObjB);
                gameObjB->OnCollision(gameObjA);
            }
        }
    }
}

void PhysicsWorld::DrawDebug()
{
    _world->debugDrawWorld();
}


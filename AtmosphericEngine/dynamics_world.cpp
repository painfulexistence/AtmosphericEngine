#include "DynamicsWorld.hpp"

DynamicsWorld::DynamicsWorld()
{
    this->_config = new btDefaultCollisionConfiguration();
    this->_dispatcher = new btCollisionDispatcher(_config);
    this->_broadphase = new btDbvtBroadphase();
    this->_solver = new btSequentialImpulseConstraintSolver();
    this->_dw = new btDiscreteDynamicsWorld(_dispatcher, _broadphase, _solver, _config);
    this->_timeAccum = 0.0f;
}

DynamicsWorld::~DynamicsWorld()
{
    delete this->_dw;
    delete this->_solver;
    delete this->_broadphase;
    delete this->_dispatcher;
    delete this->_config;
}

void DynamicsWorld::AddRigidbody(btRigidBody* rb)
{
    _dw->addRigidBody(rb);
}

void DynamicsWorld::SetConstantGravity(const float& g)
{ 
    _dw->setGravity(btVector3(0, -g, 0)); 
}

void DynamicsWorld::Step(float dt)
{
    _timeAccum += dt;
    while (_timeAccum >= FIXED_TIME_STEP) 
    {
        _dw->stepSimulation(FIXED_TIME_STEP, 0);
        _timeAccum -= FIXED_TIME_STEP;
    }
}


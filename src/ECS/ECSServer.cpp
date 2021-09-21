#include "ECS/ECSServer.hpp"
#include "ECS/EnttRegistry.hpp"

struct Transform
{

};

ECSServer::ECSServer()
{

}

ECSServer::~ECSServer()
{

}

void ECSServer::Process(float dt)
{

}

void ECSServer::OnMessage(Message msg)
{
    
}

uint64_t ECSServer::Create()
{
    uint64_t eid = this->_registry->Create();
    this->_registry->Emplace<Transform>(eid, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
    return eid;
}

void ECSServer::Destroy(uint64_t eid)
{
    this->_registry->Destroy(eid);
}

void ECSServer::SyncWithPhysics()
{

}
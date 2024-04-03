#include "ECSServer.hpp"
#include "EnttRegistry.hpp"
#include "Impostor.hpp"
#include "Camera.hpp"
#include "Renderable.hpp"
#include "Mesh.hpp"

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
    //this->_registry->Emplace<Geometry>(eid, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
    return eid;
}

void ECSServer::Destroy(uint64_t eid)
{
    this->_registry->Destroy(eid);
}

void ECSServer::SyncTransformWithPhysics()
{
    auto view = this->_registry->Data().view<Geometry&, Impostor&, Camera&>();
    view.each([this](entt::entity ent, Mesh& mesh, Impostor& impostor, Camera& camera) {
        if (this->_registry->Data().all_of<Mesh, Impostor>(ent))
            mesh.gameObject->SetModelWorldTransform(impostor.GetCenterOfMassWorldTransform());
    });
}

template<class Component, class ComponentProps> void ECSServer::AddComponent(uint64_t eid, const ComponentProps& props)
{
    this->_registry->Emplace<Component>(eid, props);
}
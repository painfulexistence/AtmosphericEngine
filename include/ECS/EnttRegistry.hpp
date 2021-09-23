#pragma once
#include "entt/entity/registry.hpp"

class EnttRegistry
{
public:
    EnttRegistry();

    ~EnttRegistry();

    uint64_t Create()
    {
        entt::entity ent = this->_registry.create();
        uint64_t eid =  this->_idCounter + 1;

        this->_lookup.insert({eid, ent});
        this->_reverseLookup.insert({ent, eid});
        this->_idCounter++;
        
        return eid;
    }

    void Destroy(uint64_t eid)
    {
        entt::entity ent = this->_lookup.find(eid)->second;
        // TODO: Delete table entries related to this entity
    }

    const entt::registry& Data()
    {
        return this->_registry;
    }

    template<class Component, class ComponentProps> void Emplace(uint64_t eid, const ComponentProps& props)
    {
        this->_registry.emplace<Component>(this->_lookup.find(eid)->second, props);
    }

    /*
    template<class Component, class... Params> void Emplace(uint64_t eid, Params&&... params)
    {
        this->_registry.emplace<Component>(this->_lookup[eid], ...params);
    }
    */

    template<class... Components> void Each(std::function<void(uint64_t, Components&&...)> entityProcessor)
    {
        //this->_registry.view<Components>().each([this](entt::entity ent, Components&&... components) {
        //    entityProcessor(this->_reverseLookup[ent], components);
        //});
    }
    
private:
    entt::registry& _registry;
    std::map<uint64_t, entt::entity> _lookup;
    std::map<entt::entity, uint64_t> _reverseLookup;
    uint64_t _idCounter = 0;
};
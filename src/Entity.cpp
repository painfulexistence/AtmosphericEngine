#include "Entity.hpp"
#include <ECS/EnttRegistry.hpp>

std::list<Entity> Entity::Entities = std::list<Entity>();

std::list<Entity> Entity::WithGraphicsComponent()
{
    std::list<Entity> ents = Entities;
    ents.remove_if([](const Entity& ent) { return (ent.GetGraphicsId() == 0); });
    return ents;
}

std::list<Entity> Entity::WithPhysicsComponent()
{
    std::list<Entity> ents = Entities;
    ents.remove_if([](const Entity& ent) { return (ent.GetPhysicsId() == 0); });
    return ents;
}

Entity::Entity()
{
    _eid = Entities.size() + 1;
}
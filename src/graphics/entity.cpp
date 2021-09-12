#include "Graphics/entity.hpp"


bool nonGraphical (const Entity& ent) { return (ent.GetGraphicsId() == 0); }

bool nonPhysical (const Entity& ent) { return (ent.GetPhysicsId() == 0); }

std::list<Entity> Entity::Entities = std::list<Entity>(0);

std::list<Entity> Entity::WithGraphicsComponent()
{
    std::list ents = Entities;
    ents.remove_if(nonGraphical);
    return ents;
}

std::list<Entity> Entity::WithPhysicsComponent()
{
    std::list ents = Entities;
    ents.remove_if(nonPhysical);
    return ents;
}

Entity::Entity()
{
    _id = Entities.size() + 1;
}
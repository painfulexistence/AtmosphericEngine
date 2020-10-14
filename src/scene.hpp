#pragma once
#include "Common.hpp"
#include "entity.hpp"

class Scene 
{
private:
    std::list<Entity*> _entities;

public:
    Scene();

    ~Scene();

    void AddChild(Entity*);

    void Render();
};
#pragma once
#include "globals.h"
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
#pragma once
#include "Common.hpp"
#include "BulletMain.h"
#include "entity.hpp"

class Terrain : public Entity
{
private:
    int _size, numVertices, numElements;

public:
    Terrain(int size, int vnum, float heightmap[]);

    ~Terrain();

    void AddRigidBody(btDiscreteDynamicsWorld*);

    void Render();
};
#pragma once

#include "globals.h"
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btTransform.h>
#include "entity.hpp"

class Cube : public Entity 
{
private:
    int numVertices = 24 * 8;
    int numElements = 36;
    int _size = 2;
public:
    GLuint ibo;

    Cube(int size);

    ~Cube();

    void AddRigidBody(glm::vec3, btDiscreteDynamicsWorld*, float);

    void Render();

    void RenderMultiple(glm::mat4*, int);
};
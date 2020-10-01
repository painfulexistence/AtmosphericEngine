#pragma once
#include "globals.h"
#include <iostream>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btTransform.h>
#include "light.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "shader.hpp"

class Terrain : public Mesh
{
private:
    int _size, numVertices, numElements;

public:
    Terrain(int size, int vnum, float heightmap[], Shader* shader);

    ~Terrain();

    void AddRigidBody(btDiscreteDynamicsWorld* dynamicsWorld);

    void Render(glm::mat4 mMatrix, Light* light, Camera* camera);
};
#pragma once

#include "globals.h"
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btTransform.h>
#include <iostream>
#include <assert.h>
#include "mesh.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "shader.hpp"

class Cube : public Mesh 
{
public:
    Cube(int size, Shader* shader);

    ~Cube();

    void AddRigidBody(glm::vec3 position, btDiscreteDynamicsWorld* dynamicsWorld, float mass);

    void Render(glm::mat4 mMatrix, Light* light, Camera* camera);

    private:
        int numVertices = 24 * 8;
        int numElements = 36;
        int _size = 2;
};
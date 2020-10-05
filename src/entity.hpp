#pragma once
#include "globals.h"
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btTransform.h>
#include "material.hpp"
#include "program.hpp"

class Entity 
{
protected:
    int _id;
    GLuint vao, vbo, ebo;
    float* vertices;
    GLushort* triangles;

    glm::mat4 _transform;
    btRigidBody* _rigidbody;


public:
    Entity()
    {
        _id = 1;
    }

    ~Entity()
    {
        _id = 0;
    }

    glm::mat4 getWorldTransform() 
    {
        btTransform trans;
        _rigidbody->getMotionState()->getWorldTransform(trans);
        btScalar mat[16] = { 0.0f };
        trans.getOpenGLMatrix(mat);
        glm::mat4 transform = glm::mat4(
            mat[0], mat[1], mat[2], mat[3],
            mat[4], mat[5], mat[6], mat[7],
            mat[8], mat[9], mat[10], mat[11],
            mat[12], mat[13], mat[14], mat[15]
        );
        return transform;
    }

    virtual void Render() { }
};
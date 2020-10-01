#pragma once
#include "globals.h"
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btTransform.h>
#include <iostream>
#include <assert.h>
#include "camera.hpp"
#include "light.hpp"
#include "shader.hpp"

class Mesh 
{
protected:
    GLuint vao, vbo, ebo;
    float* vertices;
    GLushort* triangles;
    Shader* _shader;
    btRigidBody* _rigidbody;
    glm::mat4 _localTransform;
    int _drawMode = GL_FILL;

public:
    glm::mat4 getDynamicsTransform() {
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

    void setDrawMode(int drawMode) {
        _drawMode = drawMode;
    }

    virtual void Render(glm::mat4, Light*, Camera*) {
        //
    }
};
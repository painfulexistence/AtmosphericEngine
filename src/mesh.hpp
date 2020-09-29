#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btTransform.h>
#include <iostream>
#include <assert.h>
#include "camera.hpp"
#include "light.hpp"

class Mesh {
  public:
    Mesh() {
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);
      glGenBuffers(1, &ebo);

      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }

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

    void setTexture(int texIndex) {
      _texIndex = texIndex;
    }

    void setDrawMode(int drawMode) {
      _drawMode = drawMode;
    }

    virtual void render(glm::mat4, Shader*, Light*, Camera*) {
      
    }

  protected:
    //std::vector<float> vertices;
    //std::vector<GLushort> elements;
    GLuint vao, vbo, ebo;
    btRigidBody* _rigidbody;
    glm::mat4 _localTransform;
    int _texIndex = 0;
    int _drawMode = GL_FILL;
};

#endif
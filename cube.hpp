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

class Cube {
  public:
    Cube(int size, glm::vec3 position, int texIndex, btDiscreteDynamicsWorld* dynamicsWorld = NULL, float mass = 0.f) {
      _size = size;
      _texIndex = texIndex;
      _transform = glm::scale(glm::mat4(1.0f), glm::vec3(size * 0.5f, size * 0.5f, size * 0.5f));
      /*
      for (int i = 0; i < numVertices; i++) {
        if (i % 8 == 0 && i != 0)
          std::cout << '\n';
        std::cout << vertices[i] << ' ';
      }
      */
      /*
      for (int i = 0; i < numElements; i++) {
        std::cout << elements[i] << ' ';
      }
      */

      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);
      glGenBuffers(1, &ebo);

      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      
      if (dynamicsWorld != NULL) {
        btQuaternion qtn;
        btTransform trans;
        qtn.setEuler(0, 0, 0);
        trans.setIdentity();
        trans.setRotation(qtn);
        trans.setOrigin(btVector3(position.x, position.y, position.z));
        btDefaultMotionState* motionState = new btDefaultMotionState(trans);
        btCollisionShape* shape = new btBoxShape(btVector3(size / 2.f, size / 2.f, size / 2.f));

        _rigidbody = new btRigidBody(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
        
        dynamicsWorld->addRigidBody(_rigidbody);
      }
    }

    void setTexture(int newTexIndex) {
      _texIndex = newTexIndex;
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

    void render(glm::mat4 pvmMatrix, Shader* shader) {
      //vertex data
      glBindVertexArray(vao);
      glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(float), vertices, GL_STATIC_DRAW);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, numElements*sizeof(GLushort), elements, GL_STATIC_DRAW);
      //vertex attribute
      glEnableVertexAttribArray(shader->getAttribLocation("position"));
      glEnableVertexAttribArray(shader->getAttribLocation("color"));
      glEnableVertexAttribArray(shader->getAttribLocation("uv")); 
      glVertexAttribPointer(shader->getAttribLocation("position"), 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
      glVertexAttribPointer(shader->getAttribLocation("color"), 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
      glVertexAttribPointer(shader->getAttribLocation("uv"), 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
      //uniform
      glm::mat4 mvp = pvmMatrix * _transform;
      glUniformMatrix4fv(shader->getUniformLocation("mvp"), 1, GL_FALSE, &mvp[0][0]);
      glUniform1i(shader->getUniformLocation("tex"), _texIndex);
      
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0); //glDrawArrays(GL_TRIANGLES, 0, 24);
    }

  private:
    float vertices[192] = {
      //top
      -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
      //bottom
      -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
      1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
      //left
      -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      //right
      1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      //front
      1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
      -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
      -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
      //back
      1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
      -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f
    };
    GLushort elements[36] = {
      0, 1, 3,
      1, 2, 3,
      4, 5, 7,
      5, 6, 7,
      8, 9, 10,
      9, 10, 11,
      12, 13, 14,
      13, 14, 15,
      16, 17, 18,
      17, 18, 19,
      20, 21, 22,
      21, 22, 23
    };
    int numVertices = 24 * 8;
    int numElements = 36;

    GLuint vao, vbo, ebo;

    glm::mat4 _transform;
    btRigidBody* _rigidbody;

    int _size = 2;
    int _texIndex = 0;
};

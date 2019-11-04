#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btTransform.h>

class Terrain {
  public:
    Terrain(int size, int vnum, float heightmap[], btDiscreteDynamicsWorld* dynamicsWorld = NULL) {
      numVertices = vnum * vnum * 8;
      vertices = new float[numVertices];
      float c_size = float(size)/float(vnum-1);
      for (int i = 0; i < vnum; i++) {
        for (int j = 0; j < vnum; j++) {
          float x = - float(size/2) + j * c_size;
          float y = heightmap[(i * vnum + j)];
          float z = - float(size/2) + i * c_size;
          vertices[(i * vnum + j) * 8 + 0] = x;
          vertices[(i * vnum + j) * 8 + 1] = y;
          vertices[(i * vnum + j) * 8 + 2] = z;
          vertices[(i * vnum + j) * 8 + 3] = 2 * float(i)/float(vnum);
          vertices[(i * vnum + j) * 8 + 4] = 2 * float(j)/float(vnum);
          vertices[(i * vnum + j) * 8 + 5] = 0.8f;
          vertices[(i * vnum + j) * 8 + 6] = (float)(i%2);
          vertices[(i * vnum + j) * 8 + 7] = (float)(j%2);
        }
      }
      /*
      for (int i = 0; i < numVertices; i++) {
        if (i % 8 == 0 && i != 0)
          std::cout << '\n';
        std::cout << vertices[i] << ' ';
      }
      */
      numElements = (vnum * 2 + 1) * (vnum - 1);
      elements = new GLushort[numElements];
      for (int i = 0; i < vnum - 1; i++) {
        for (int j = 0; j < 2 * vnum; j++) {
          elements[i * (2 * vnum + 1) + j] = (i + j % 2) * vnum + (j / 2);
        }
        elements[i * (2 * vnum + 1) + 2 * vnum] = 0xFFFF;
      }
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
        trans.setOrigin(btVector3(0, -5, 0));
        btDefaultMotionState* motionState = new btDefaultMotionState(trans);
        btCollisionShape* shape = new btBoxShape(btVector3(size / 2.f, 5, size / 2.f));

        _rigidbody = new btRigidBody(btScalar(0.0), motionState, shape, btVector3(1, 1, 1));
        dynamicsWorld->addRigidBody(_rigidbody);
      }
    }
    ~Terrain() {
      delete[] vertices;
      delete[] elements;
    }

    void render(glm::mat4 pvmMatrix, int drawMode, Shader* shader) {

      glBindVertexArray(vao);
      glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(float), vertices, GL_STATIC_DRAW);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, numElements*sizeof(GLushort), elements, GL_STATIC_DRAW);

      glEnableVertexAttribArray(shader->getAttribLocation("position"));
      glEnableVertexAttribArray(shader->getAttribLocation("color"));
      glEnableVertexAttribArray(shader->getAttribLocation("uv")); 
      glVertexAttribPointer(shader->getAttribLocation("position"), 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
      glVertexAttribPointer(shader->getAttribLocation("color"), 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
      glVertexAttribPointer(shader->getAttribLocation("uv"), 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));

      glm::mat4 mvp = pvmMatrix * _transform;
      glUniformMatrix4fv(shader->getUniformLocation("mvp"), 1, GL_FALSE, &mvp[0][0]);
      glUniform1i(shader->getUniformLocation("tex"), 1);

      glPolygonMode(GL_FRONT_AND_BACK, drawMode);
      glDrawElements(GL_TRIANGLE_STRIP, numElements, GL_UNSIGNED_SHORT, 0);
    }

  private:
    float* vertices;
    GLushort* elements;
    int numVertices, numElements;
    GLuint vao, vbo, ebo;

    glm::mat4 _transform = glm::mat4(1.0f);
    btRigidBody* _rigidbody;
};

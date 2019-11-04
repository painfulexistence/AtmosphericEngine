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

      shaderProgram = glCreateProgram();
      const char* vertexSrc = R"glsl(
          #version 330

          in vec3 position;
          in vec3 color;
          in vec2 uv;
          out vec2 tex_uv;
          out vec3 tex_hue;
          uniform mat4 mvp;

          void main()
          {
              tex_uv = uv;
              tex_hue = color;
              gl_Position = mvp * vec4(position, 1.0);
          }
      )glsl";
      const char* fragmentSrc = R"glsl(
          #version 330

          in vec2 tex_uv;
          in vec3 tex_hue;
          out vec4 outColor;

          uniform sampler2D tex;

          void main()
          {
              outColor = texture(tex, tex_uv) * vec4(tex_hue, 1.0);
          }
      )glsl";
      GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertexShader, 1, &vertexSrc, NULL);
      glCompileShader(vertexShader);
      GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
      glCompileShader(fragmentShader);
      glAttachShader(shaderProgram, vertexShader);
      glAttachShader(shaderProgram, fragmentShader);
      glLinkProgram(shaderProgram);

      posAttrib = glGetAttribLocation(shaderProgram, "position");
      colorAttrib = glGetAttribLocation(shaderProgram, "color");
      uvAttrib = glGetAttribLocation(shaderProgram, "uv");
      mvpUniform = glGetUniformLocation(shaderProgram, "mvp");
      tex = glGetUniformLocation(shaderProgram, "tex");

      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);
      glGenBuffers(1, &ebo);
      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glEnableVertexAttribArray(posAttrib);
      glEnableVertexAttribArray(colorAttrib);
      glEnableVertexAttribArray(uvAttrib);

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

    void render(glm::mat4 pvmMatrix, int drawMode) {
      glUseProgram(shaderProgram);
      glPolygonMode(GL_FRONT_AND_BACK, drawMode);
      
      glm::mat4 mvp = pvmMatrix * _transform;
      glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, &mvp[0][0]);
      glUniform1i(tex, 1);
      glBindVertexArray(vao);
      glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(float), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
      glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
      glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, numElements*sizeof(GLushort), elements, GL_STATIC_DRAW);
      glDrawElements(GL_TRIANGLE_STRIP, numElements, GL_UNSIGNED_SHORT, 0);
    }

  private:
    float* vertices;
    GLushort* elements;
    int numVertices, numElements;
    GLuint shaderProgram, vao, vbo, ebo;
    GLint posAttrib, colorAttrib, uvAttrib, mvpUniform, tex;

    glm::mat4 _transform = glm::mat4(1.0f);
    btRigidBody* _rigidbody;
};

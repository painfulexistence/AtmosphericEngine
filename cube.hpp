#include <GL/glew.h>
#include <iostream>

class Cube {
  public:
    Cube(int size) {
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
    }
    float* getVertexArray() {
      return vertices;
    }
    GLushort* getElementArray() {
      return elements;
    }
    int countVertices() {
      return numVertices;
    }
    int countElements() {
      return numElements;
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
    float strip_vertices[42] = {
      -1.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, -1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f, 1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, -1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f
    };
};

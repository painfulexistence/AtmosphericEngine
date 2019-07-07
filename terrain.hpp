#include <GL/glew.h>
#include <iostream>

class Terrain {
  public:
    Terrain(int size, int vnum, float heightmap[]) {
      numVertices = vnum * vnum * 8;
      vertices = new float[numVertices];
      float c_size = float(size)/float(vnum-1);
      for (int i = 0; i < vnum; i++) {
        for (int j = 0; j < vnum; j++) {
          vertices[(i * vnum + j) * 8 + 0] = - float(size/2) + j * c_size;
          vertices[(i * vnum + j) * 8 + 1] = heightmap[(i * vnum + j)];
          vertices[(i * vnum + j) * 8 + 2] = - float(size/2) + i * c_size;
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
    }
    ~Terrain() {
      delete[] vertices;
      delete[] elements;
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
    float* vertices;
    GLushort* elements;
    int numVertices;
    int numElements;
};

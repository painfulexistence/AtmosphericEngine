#pragma once
#include "globals.hpp"

struct Material
{
    int baseMap = -1;
    int normalMap = -1;
    int aoMap = -1;
    int roughnessMap = -1;
    int metallicMap = -1;
    int heightMap = -1;
    glm::vec3 diffuse = glm::vec3(.55, .55, .55);
    glm::vec3 specular = glm::vec3(.7, .7, .7);
    glm::vec3 ambient = glm::vec3(0, 0, 0);
    float shininess = .25;
    bool cullFaceEnabled = true;
    GLenum primitiveType = GL_TRIANGLES;
    GLenum polygonMode = GL_FILL;
};
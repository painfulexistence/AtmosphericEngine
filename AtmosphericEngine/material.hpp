#pragma once
#include "globals.hpp"

struct MaterialProps {
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
#ifndef __EMSCRIPTEN__
    GLenum polygonMode = GL_FILL;
#endif
};

class Material {
public:
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
#ifndef __EMSCRIPTEN__
    GLenum polygonMode = GL_FILL;
#endif

    Material(const MaterialProps& props) {
        baseMap = props.baseMap;
        normalMap = props.normalMap;
        aoMap = props.aoMap;
        roughnessMap = props.roughnessMap;
        metallicMap = props.metallicMap;
        heightMap = props.heightMap;
        diffuse = props.diffuse;
        specular = props.specular;
        ambient = props.ambient;
        shininess = props.shininess;
        cullFaceEnabled = props.cullFaceEnabled;
        primitiveType = props.primitiveType;
    #ifndef __EMSCRIPTEN__
        polygonMode = props.polygonMode;
    #endif
    }
};
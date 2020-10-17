#pragma once
#include "../common.hpp"

class Material 
{
private:
    std::string _key;
    GLuint _mainTex;
    glm::vec3 _ambient;
    glm::vec3 _diffuse;
    glm::vec3 _specular;
    float _shininess;

public:
    Material(std::string, GLuint, glm::vec3, glm::vec3, glm::vec3, float);

    ~Material();

    std::string GetKey() { return _key; }

    GLuint GetMainTex() { return _mainTex; }

    glm::vec3 GetAmbient() { return _ambient; }

    glm::vec3 GetDiffuse() { return _diffuse; }

    glm::vec3 GetSpecular() { return _specular; }

    float GetShininess() { return _shininess; }
};

#pragma once
#include "../common.hpp"

class Material 
{
private:
    std::string _key;
    int _texIdx;
    glm::vec3 _ambient;
    glm::vec3 _diffuse;
    glm::vec3 _specular;
    float _shininess;

public:
    Material(std::string, int, glm::vec3, glm::vec3, glm::vec3, float);

    ~Material();

    std::string GetKey() { return _key; }

    int GetTexUnit() { return NUM_MAP_TEXS + _texIdx; }

    glm::vec3 GetAmbient() { return _ambient; }

    glm::vec3 GetDiffuse() { return _diffuse; }

    glm::vec3 GetSpecular() { return _specular; }

    float GetShininess() { return _shininess; }
};

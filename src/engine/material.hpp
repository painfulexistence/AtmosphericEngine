#pragma once
#include "../common.hpp"

class Material 
{
private:
    int _texIdx;
    glm::vec3 _ambient;
    glm::vec3 _diffuse;
    glm::vec3 _specular;
    float _shininess;

public:
    static Material Pearl;
    static Material Night;
    static Material Ivory;
    static Material GreenPlastic;

    Material(
        int texIdx = 1, 
        glm::vec3 ambient = glm::vec3(0, 0, 0), 
        glm::vec3 diffuse = glm::vec3(.55, .55, .55), 
        glm::vec3 specular = glm::vec3(.7, .7, .7), 
        float shininess = .25
    );

    int GetTexUnit() const { return NUM_MAP_UNITS + _texIdx; }

    glm::vec3 GetAmbient() const { return _ambient; }

    glm::vec3 GetDiffuse() const { return _diffuse; }

    glm::vec3 GetSpecular() const { return _specular; }

    float GetShininess() const { return _shininess; }
};
#pragma once
#include "../common.hpp"

enum LightType
{
    POINT,
    AREA,
    DIRECT,
    SPOT
};

struct LightProperties
{
    LightType type;
    glm::vec3 ambient = glm::vec3(0.8);
    glm::vec3 diffuse;
    glm::vec3 specular = glm::vec3(1);
};

class Light 
{
private:
    glm::vec3 _position;
    LightType _type;
    glm::vec3 _ambient;
    glm::vec3 _diffuse;
    glm::vec3 _specular;
    glm::vec3 _direction;

public:
    Light(glm::vec3, LightProperties, glm::vec3 = glm::vec3(0, 0, 0));
    
    void SetPosition(glm::vec3);

    void SetDirection(glm::vec3);
    
    glm::vec3 GetPosition() { return _position; }

    glm::vec3 GetDirection() { return _direction; }

    glm::vec3 GetAmbient() { return _ambient; }

    glm::vec3 GetDiffuse() { return _diffuse; }

    glm::vec3 GetSpecular() { return _specular; }

    void SetAmbient(glm::vec3 ambient) { _ambient = ambient; }

    void SetDiffuse(glm::vec3 diffuse) { _diffuse = diffuse; }

    void SetSpecular(glm::vec3 specular) { _specular = specular; }
};
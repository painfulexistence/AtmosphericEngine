#pragma once
#include "globals.h"
#include <iostream>
#include <assert.h>

class Light 
{
private:
    glm::vec3 _position;
    glm::vec3 _direction;
    glm::vec3 _ambient;
    glm::vec3 _diffuse;
    glm::vec3 _specular;

public:
    Light(glm::vec3, glm::vec3);
    
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
#pragma once
#include "../common.hpp"

struct LightProperties
{
    //Color reference: http://planetpixelemporium.com/tutorialpages/light.html
    //Attenuation reference: http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    glm::vec3 ambient = glm::vec3(0.2);
    glm::vec3 diffuse = glm::vec3(0.5, 0.5, 0.5);
    glm::vec3 specular = glm::vec3(1);
    glm::vec3 direction = glm::vec3(0, -1, 0);
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 attenuation = glm::vec3(1, 0.045, 0.0075);
    float intensity = 1.0;
};

class Light 
{
private:
    int _type;
    glm::vec3 _direction;
    glm::vec3 _position;
    glm::vec3 _attenuation;
    glm::vec3 _ambient;
    glm::vec3 _diffuse;
    glm::vec3 _specular;
    float _intensity;

public:
    Light(LightProperties = LightProperties(), int = POINT_LIGHT);

    glm::mat4 GetProjectionViewMatrix();
    
    glm::vec3 GetPosition() { return _position; }

    glm::vec3 GetDirection() { return _direction; }

    glm::vec3 GetAmbient() { return _ambient; }

    glm::vec3 GetDiffuse() { return _diffuse; }

    glm::vec3 GetSpecular() { return _specular; }

    glm::vec3 GetAttenuation() { return _attenuation; }

    float GetIntensity() { return _intensity; }

    void SetPosition(glm::vec3 position) 
    {
        _position = position;
    }

    void SetDirection(glm::vec3 direction) 
    {
        _direction = direction;
    }

    void SetAmbient(glm::vec3 ambient) 
    { 
        _ambient = ambient; 
    }

    void SetDiffuse(glm::vec3 diffuse) 
    { 
        _diffuse = diffuse; 
    }

    void SetSpecular(glm::vec3 specular) 
    { 
        _specular = specular; 
    }
};
#include "light.hpp"

Light::Light(glm::vec3 position, glm::vec3 diffuse) 
{
    _position = position;
    _direction = glm::vec3(1, -1, 1);
    _ambient = glm::vec3(0.8);
    _diffuse = diffuse;
    _specular = glm::vec3(1);
}

void Light::SetPosition(glm::vec3 position) 
{
    _position = position;
}

void Light::SetDirection(glm::vec3 direction) 
{
    _direction = direction;
}
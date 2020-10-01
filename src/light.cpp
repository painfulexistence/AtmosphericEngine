#include "light.hpp"

Light::Light(glm::vec3 position, glm::vec3 color) 
{
    _position = position;
    _color = color;
    _direction = glm::vec3(0, 0, 0);
}

void Light::setPosition(glm::vec3 position) 
{
    _position = position;
}

void Light::setColor(glm::vec3 color) 
{
    _color = color;
}

void Light::setDirection(glm::vec3 direction) 
{
    _direction = direction;
}
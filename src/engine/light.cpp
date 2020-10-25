#include "light.hpp"

Light::Light(glm::vec3 position, LightProperties props, glm::vec3 direction) 
{
    _position = position;
    _type = props.type;
    _ambient = props.ambient;
    _diffuse = props.diffuse;
    _specular = props.specular;
    _direction = direction;
}

void Light::SetPosition(glm::vec3 position) 
{
    _position = position;
}

void Light::SetDirection(glm::vec3 direction) 
{
    _direction = direction;
}
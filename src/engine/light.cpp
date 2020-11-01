#include "light.hpp"

Light::Light(LightProperties props, int type) : _type(type)
{
    _position = props.position;
    _direction = props.direction;
    _ambient = props.ambient;
    _diffuse = props.diffuse;
    _specular = props.specular;
    _intensity = props.intensity;
    _attenuation = props.attenuation;
}

glm::mat4 Light::GetProjectionViewMatrix()    
{
    if (_type == DIR_LIGHT)
    {
        float nearPlane = -200.0f, farPlane = 200.0f;
        glm::mat4 projection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, nearPlane, farPlane);  
        glm::mat4 view = glm::lookAt(-GetDirection(), glm::vec3(0.f), glm::vec3(0, 1, 0));
        return projection * view;
    }
    else
    {
        std::cout << "Omnidirectional light shadow not supported!" << std::endl;
        return glm::mat4(0.0f);
    }
}

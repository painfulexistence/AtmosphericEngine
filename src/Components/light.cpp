#include "Components/light.hpp"
#include "Components/GameObject.hpp"

static glm::vec3 Direction(GLenum face)
{
    switch (face)
    {
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            return glm::vec3(1.f, 0.f, 0.f);
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            return glm::vec3(-1.f, 0.f, 0.f);
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            return glm::vec3(0.f, 1.f, 0.f);
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            return glm::vec3(0.f, -1.f, 0.f);
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            return glm::vec3(0.f, 0.f, 1.f);
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            return glm::vec3(0.f, 0.f, -1.f);
        default:
            std::runtime_error("Invalid cube face");
            return glm::vec3(1.f, 0.f, 0.f);
    }
}

static glm::vec3 WorldUp(GLenum face)
{
    switch (face)
    {
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            return glm::vec3(0.f, -1.f, 0.f);
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            return glm::vec3(0.f, -1.f, 0.f);
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            return glm::vec3(0.f, 0.f, -1.f);
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            return glm::vec3(0.f, 0.f, 1.f);
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            return glm::vec3(0.f, -1.f, 0.f);
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            return glm::vec3(0.f, -1.f, 0.f);
        default:
            std::runtime_error("Invalid cube face");
            return glm::vec3(0.f, -1.f, 0.f);
    }
}

Light::Light(GameObject* gameObject, LightProps props)
{
    type = props.type;
    position = props.position;
    direction = props.direction;
    ambient = props.ambient;
    diffuse = props.diffuse;
    specular = props.specular;
    intensity = props.intensity;
    attenuation = props.attenuation;
    castShadow = props.castShadow;

    this->gameObject = gameObject;
    this->gameObject->AddComponent(this);
}

std::string Light::GetName() const
{
    return std::string("Light");
}

glm::mat4 Light::GetProjectionMatrix(int cascadedIndex)    
{
    if (type == DIR_LIGHT)
    {
        float nearZ = -200.0f, farZ = 200.0f;
        glm::mat4 view = glm::lookAt(-glm::normalize(direction), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f)); //NOTES: should be placed as far as possible, but here
        return glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, nearZ, farZ);  
    }
    else
    {
        //NOTES:
        // Here a small farZ would be used to avoid unnecessary shadow calculation of cutoff point lights
        float nearZ = 0.1f, farZ = 400.0f;
        return glm::perspective(glm::radians(90.0f), 1.0f, nearZ, farZ);
    }
}

glm::mat4 Light::GetViewMatrix(GLenum facing)    
{
    if (type == DIR_LIGHT)
    {
        //NOTES:
        // To simulate real world lighting, directional light should be placed as far as possible,
        // but here the light would be placed depending on farZ to ensure the scene can completely fit itself into clip space
        return glm::lookAt(-glm::normalize(direction), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
    }
    else
    {
        return glm::lookAt(position, position + Direction(facing), WorldUp(facing));
    }
}

glm::mat4 Light::GetProjectionViewMatrix(int cascadedIndex, GLenum face)    
{
    if (type == DIR_LIGHT)
    {
        float nearZ = -200.0f, farZ = 200.0f;
        glm::mat4 projection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, nearZ, farZ);  
        glm::mat4 view = glm::lookAt(-glm::normalize(direction), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
        return projection * view;
    }
    else
    {
        float nearZ = 0.1f, farZ = 1000.0f;
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, nearZ, farZ);
        glm::mat4 view = glm::lookAt(position, position + Direction(face), WorldUp(face));
        return projection * view;
    }
}

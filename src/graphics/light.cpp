#include "Graphics/light.hpp"

Light::Light(sol::table t)
{
    type = (int)t.get_or("type", 1);
    position = glm::vec3(t["position"][1], t["position"][2], t["position"][3]);
    direction = glm::vec3(t["direction"][1], t["direction"][2], t["direction"][3]);
    ambient = glm::vec3(t["ambient"][1], t["ambient"][2], t["ambient"][3]);
    diffuse = glm::vec3(t["diffuse"][1], t["diffuse"][2], t["diffuse"][3]);
    specular = glm::vec3(t["specular"][1], t["specular"][2], t["specular"][3]);
    attenuation = glm::vec3(t["attenuation"][1], t["attenuation"][2], t["attenuation"][3]);
    intensity = (float)t.get_or("intensity", 1.0);
    castShadow = (int)t.get_or("castShadow", 0);

}

Light::Light(LightProperties props, int type) : type(type)
{
    position = props.position;
    direction = props.direction;
    ambient = props.ambient;
    diffuse = props.diffuse;
    specular = props.specular;
    intensity = props.intensity;
    attenuation = props.attenuation;
    castShadow = props.castShadow;
}

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

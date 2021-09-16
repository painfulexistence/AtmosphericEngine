#pragma once
#include "common.hpp"
#include "Scripting/Lua.hpp"

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
    bool castShadow = false;
};

struct Light 
{
    int type;
    glm::vec3 direction;
    glm::vec3 position;
    glm::vec3 attenuation;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float intensity;
    int castShadow;

    Light(sol::table);
    
    Light(LightProperties props, int type = POINT_LIGHT);

    glm::mat4 GetProjectionMatrix(int cascadedIndex = 0);

    glm::mat4 GetViewMatrix(GLenum facing = GL_TEXTURE_CUBE_MAP_POSITIVE_X);

    glm::mat4 GetProjectionViewMatrix(int cascadedIndex = 0, GLenum facing = GL_TEXTURE_CUBE_MAP_POSITIVE_X);
};
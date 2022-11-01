#pragma once
#include "Globals.hpp"
#include "Lua.hpp"
#include "Component.hpp"

struct LightProps
{
    LightProps(sol::table data)
    {
        //Color reference: http://planetpixelemporium.com/tutorialpages/light.html
        //Attenuation reference: http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
        this->type = (int)data.get_or("type", 1);
        this->position = glm::vec3(
            data["position"][1], 
            data["position"][2], 
            data["position"][3]
        );
        this->direction = glm::vec3(
            data["direction"][1], 
            data["direction"][2], 
            data["direction"][3]
        );
        this->ambient = glm::vec3(
            data["ambient"][1], 
            data["ambient"][2], 
            data["ambient"][3]
        );
        this->diffuse = glm::vec3(
            data["diffuse"][1], 
            data["diffuse"][2], 
            data["diffuse"][3]
        );
        this->specular = glm::vec3(
            data["specular"][1], 
            data["specular"][2], 
            data["specular"][3]
        );
        this->attenuation = glm::vec3(
            data["attenuation"][1], 
            data["attenuation"][2], 
            data["attenuation"][3]
        );
        this->intensity = (float)data.get_or("intensity", 1.0);
        this->castShadow = (bool)data.get_or("castShadow", 0);
    };
    int type;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 direction;
    glm::vec3 position;
    glm::vec3 attenuation;
    float intensity;
    int castShadow;
};

class Light : public Component
{
public:
    int type;
    glm::vec3 direction;
    glm::vec3 position;
    glm::vec3 attenuation;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float intensity;
    int castShadow;
    
    Light(GameObject* gameObject, LightProps props);

    std::string GetName() const override;

    glm::mat4 GetProjectionMatrix(int cascadedIndex = 0);

    glm::mat4 GetViewMatrix(GLenum facing = GL_TEXTURE_CUBE_MAP_POSITIVE_X);

    glm::mat4 GetProjectionViewMatrix(int cascadedIndex = 0, GLenum facing = GL_TEXTURE_CUBE_MAP_POSITIVE_X);
};
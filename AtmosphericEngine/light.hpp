#pragma once
#include "globals.hpp"
#include "component.hpp"

struct LightProps
{
    //Color reference: http://planetpixelemporium.com/tutorialpages/light.html
    //Attenuation reference: http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
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
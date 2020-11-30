#pragma once
#include "../common.hpp"

struct Material 
{
    int textureIdx;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    Material(sol::table t);
    
    Material(
        int textureIdx = 1, 
        glm::vec3 ambient = glm::vec3(0, 0, 0), 
        glm::vec3 diffuse = glm::vec3(.55, .55, .55), 
        glm::vec3 specular = glm::vec3(.7, .7, .7), 
        float shininess = .25
    );

    int GetTexUnit() const { return NUM_MAP_UNITS + textureIdx; }
};
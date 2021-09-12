#pragma once
#include "../common.hpp"

struct Material 
{
    int textureIdx;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float ao;

    Material(sol::table t);
    
    Material(
        int textureIdx = 1, 
        glm::vec3 ambient = glm::vec3(0, 0, 0), 
        glm::vec3 diffuse = glm::vec3(.55, .55, .55), 
        glm::vec3 specular = glm::vec3(.7, .7, .7), 
        float shininess = .25,
        glm::vec3 albedo = glm::vec3(.55, .55, .55),
        float metallic = 1.f,
        float roughness = 0.5f,
        float ao = 1.f
    );

    int GetTexUnit() const { return NUM_MAP_UNITS + textureIdx; }
};
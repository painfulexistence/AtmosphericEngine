#pragma once
#include "globals.hpp"
#include "script.hpp"

struct Material
{
    int baseMap;
    int normalMap;
    int aoMap;
    int roughnessMap;
    int metallicMap;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 ambient;
    float shininess;

    Material(sol::table t)
    {
        baseMap = (int)t.get_or("baseMapId", -1);
        normalMap = (int)t.get_or("normalMapId", -1);
        aoMap = (int)t.get_or("aoMapId", -1);
        roughnessMap = (int)t.get_or("roughtnessMapId", -1);
        metallicMap = (int)t.get_or("metallicMapId", -1);
        diffuse = glm::vec3(t["diffuse"][1], t["diffuse"][2], t["diffuse"][3]);
        specular = glm::vec3(t["specular"][1], t["specular"][2], t["specular"][3]);
        ambient = glm::vec3(t["ambient"][1], t["ambient"][2], t["ambient"][3]);
        shininess = (float)t.get_or("shininess", 0.25);
    };

    Material(
        int baseMap = -1,
        int normalMap = -1,
        int aoMap = -1,
        int roughnessMap = -1,
        int metallicMap = -1,
        glm::vec3 diffuse = glm::vec3(.55, .55, .55),
        glm::vec3 specular = glm::vec3(.7, .7, .7),
        glm::vec3 ambient = glm::vec3(0, 0, 0),
        float shininess = .25
    ) : baseMap(baseMap), normalMap(normalMap),
    ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {};
};
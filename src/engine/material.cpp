#include "material.hpp"

Material::Material(sol::table t)
{
    textureIdx = (int)t.get_or("textureIdx", 1);
    ambient = glm::vec3(t["ambient"][1], t["ambient"][2], t["ambient"][3]);
    diffuse = glm::vec3(t["diffuse"][1], t["diffuse"][2], t["diffuse"][3]);
    specular = glm::vec3(t["specular"][1], t["specular"][2], t["specular"][3]);
    shininess = (float)t.get_or("shininess", 0.25);
}

Material::Material(
    int textureIdx, 
    glm::vec3 ambient, 
    glm::vec3 diffuse, 
    glm::vec3 specular, 
    float shininess
) : textureIdx(textureIdx), ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {}


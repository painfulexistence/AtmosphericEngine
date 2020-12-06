#include "material.hpp"

Material::Material(sol::table t)
{
    textureIdx = (int)t.get_or("textureIdx", 1);
    ambient = glm::vec3(t["ambient"][1], t["ambient"][2], t["ambient"][3]);
    diffuse = glm::vec3(t["diffuse"][1], t["diffuse"][2], t["diffuse"][3]);
    specular = glm::vec3(t["specular"][1], t["specular"][2], t["specular"][3]);
    shininess = (float)t.get_or("shininess", 0.25);
    albedo = glm::vec3(t["albedo"][1], t["albedo"][2], t["albedo"][3]);
    metallic = (float)t.get_or("metallic", 1.0f);
    roughness = (float)t.get_or("roughness", 0.5f);
    ao = (float)t.get_or("ao", 0.1f);
}

Material::Material(
    int textureIdx, 
    glm::vec3 ambient, 
    glm::vec3 diffuse, 
    glm::vec3 specular, 
    float shininess,
    glm::vec3 albedo,
    float metallic,
    float roughness,
    float ao
) : textureIdx(textureIdx), 
    ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess),
    albedo(albedo), metallic(metallic), roughness(roughness), ao(ao) {}


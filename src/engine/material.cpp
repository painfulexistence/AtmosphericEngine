#include "material.hpp"

Material::Material(std::string key, GLuint mainTex, glm::vec3 ambient = glm::vec3(0, 0, 0), glm::vec3 diffuse = glm::vec3(.55, .55, .55), glm::vec3 specular = glm::vec3(.7, .7, .7), float shininess = .25) 
{
    _key = key;
    _mainTex = mainTex;
    _ambient = ambient;
    _diffuse = diffuse;
    _specular = specular;
    _shininess = shininess;
}

Material::~Material() {}

#include "material.hpp"

Material::Material(int texIdx, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess) 
{
    _texIdx = texIdx;
    _ambient = ambient;
    _diffuse = diffuse;
    _specular = specular;
    _shininess = shininess;
}

Material Material::Pearl = Material(
    (GLuint)4,
    glm::vec3(.25, .20725, .20725),
    glm::vec3(1, .829, .829),
    glm::vec3(.296648, .296648, .296648),
    0.088
);

Material Material::Night = Material(
    (GLuint)1,
    glm::vec3(.1, .18725, .1745),
    glm::vec3(.396, .74151, .69102),
    glm::vec3(.992157, .941176, .807843),
    0.1
);

Material Material::Ivory = Material(
    (GLuint)4,
    glm::vec3(0.19225, 0.19225, 0.19225),
    glm::vec3(0.50754, 0.50754, 0.50754),
    glm::vec3(0.508273, 0.508273, 0.508273),
    0.4
);

Material Material::GreenPlastic = Material(
    (GLuint)3,
    glm::vec3(.0, .0, .0),
    glm::vec3(.5, .0, .0),
    glm::vec3(.7, .6, .6),
    0.25
);
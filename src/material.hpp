#pragma once
#include "globals.h"
#include "program.hpp"

class Material 
{
private:
    Program* _program;
    GLuint _mainTex;
    glm::vec3 _ambient;
    glm::vec3 _diffuse;
    glm::vec3 _specular;
    float _shininess;

public:
    Material(Program*, GLuint, glm::vec3, glm::vec3, glm::vec3, float);

    ~Material();

    Program* GetProgram() { return _program; }

    GLuint GetMainTex() { return _mainTex; }

    glm::vec3 GetAmbient() { return _ambient; }

    glm::vec3 GetDiffuse() { return _diffuse; }

    glm::vec3 GetSpecular() { return _specular; }

    float GetShininess() { return _shininess; }
};
#pragma once
#include "../common.hpp"
#include "shader.hpp"

class Program 
{
private:
    GLuint _program;

public:
    Program();
    
    void Init();

    GLint GetAttrib(const char* attrib);

    GLint GetUniform(const char* uniform);

    void Activate();

    void Deactivate();
};

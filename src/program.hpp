#pragma once
#include "Common.hpp"
#include "shader.hpp"

class Program 
{
private:
    GLuint _program;
    Shader* _shaders;

public:
    Program(std::vector<Shader*>);
    
    ~Program();

    GLint GetAttrib(const char* attrib);

    GLint GetUniform(const char* uniform);

    void Activate();

    void Deactivate();
};
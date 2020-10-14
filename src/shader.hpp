#pragma once
#include "Common.hpp"

class Shader 
{
private:
    GLuint _shader;

public:
    Shader(const char*, GLenum);
    
    ~Shader();

    GLuint Compile();
};
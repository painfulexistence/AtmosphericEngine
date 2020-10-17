#pragma once
#include "../common.hpp"

class Shader 
{
private:
    GLuint _shader;

public:
    Shader(const char*, GLenum);
    
    ~Shader();

    GLuint Compile();
};
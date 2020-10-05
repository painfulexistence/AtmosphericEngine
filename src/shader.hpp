#pragma once
#include "globals.h"
#include <fstream>
#include <string>

class Shader 
{
private:
    GLuint _shader;

public:
    Shader(const char*, GLenum);
    
    ~Shader();

    GLuint Compile();
};
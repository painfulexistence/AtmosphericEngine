#pragma once
#include "../common.hpp"

struct Shader 
{
    GLuint shader;

    Shader(const std::string& path, GLenum type);    
};
#pragma once
#include "../common.hpp"
#include "../io/file.hpp"

class Shader 
{
private:
    GLuint _shader;

public:
    Shader(const std::string& filename, GLenum type);
    
    GLuint Compile();
};
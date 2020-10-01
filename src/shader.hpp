#pragma once
#include <GL/glew.h>
#include <fstream>
#include <string>

#define NUM_SHADERS 4
#define SHADER_V 0
#define SHADER_F 1
#define SHADER_2D_V 2
#define SHADER_2D_F 3

class Shader 
{
private:
    GLuint program;
    GLuint shaders[NUM_SHADERS];

public:
    Shader() {}
    
    ~Shader() {}

    void Load();

    void Attach(int shader_id);

    void Detach(int shader_id);

    void Activate();

    void Deactivate();

    GLint GetAttrib(const char* attrib);

    GLint GetUniform(const char* uniform);
};
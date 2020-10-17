#include "program.hpp"

Program::Program()
{
    _program = glCreateProgram();
}

void Program::Init()
{
    std::vector<Shader*> shaders = {
        new Shader("./resources/shaders/normal.vert", GL_VERTEX_SHADER),
        new Shader("./resources/shaders/glitched.frag", GL_FRAGMENT_SHADER)
    };
    for (int i = shaders.size() - 1; i >= 0; i--)
    {
        glAttachShader(_program, shaders[i]->Compile());
        delete shaders[i];
    }

    glLinkProgram(_program);
}

GLint Program::GetAttrib(const char* attrib) {
    return glGetAttribLocation(_program, attrib);
}

GLint Program::GetUniform(const char* uniform) {
    return glGetUniformLocation(_program, uniform);
}

void Program::Activate() {
    glUseProgram(_program);
}

void Program::Deactivate() {
    glUseProgram(0);
}

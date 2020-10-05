#include "program.hpp"

Program::Program(std::vector<Shader*> shaders)
{
    _program = glCreateProgram();
    for (int i = 0; i < shaders.size(); i++)
    {
        glAttachShader(_program, shaders[i]->Compile());
    }
    glLinkProgram(_program);
}

Program::~Program() {}

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
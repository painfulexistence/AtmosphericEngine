#include "program.hpp"

Program::Program() : _program(glCreateProgram()) {}

Program::Program(std::vector<Shader>& shaders) : _program(glCreateProgram())
{
    for (int i = shaders.size() - 1; i >= 0; i--)
    {
        glAttachShader(_program, shaders[i].Compile());
    }
    glLinkProgram(_program);
}

void Program::Activate() {
    glUseProgram(_program);
}

void Program::Deactivate() {
    glUseProgram(0);
}

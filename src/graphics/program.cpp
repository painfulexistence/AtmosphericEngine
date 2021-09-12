#include "Graphics/program.hpp"

ShaderProgram::ShaderProgram() {}

ShaderProgram::ShaderProgram(std::string vert, std::string frag) : program(glCreateProgram())
{
    glAttachShader(program, Shader(vert, GL_VERTEX_SHADER).shader);
    glAttachShader(program, Shader(frag, GL_FRAGMENT_SHADER).shader);
    glLinkProgram(program);
}

ShaderProgram::ShaderProgram(std::vector<Shader>& shaders) : program(glCreateProgram())
{
    for (int i = shaders.size() - 1; i >= 0; i--)
    {
        glAttachShader(program, shaders[i].shader);
    }
    glLinkProgram(program);
}

void ShaderProgram::Activate() {
    glUseProgram(program);
}

void ShaderProgram::Deactivate() {
    glUseProgram(0);
}

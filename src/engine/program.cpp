#include "program.hpp"

ShaderProgram::ShaderProgram() {}

ShaderProgram::ShaderProgram(sol::table t) : program(glCreateProgram())
{
    glAttachShader(program, Shader(std::string(t["vert"]), GL_VERTEX_SHADER).shader);
    glAttachShader(program, Shader(std::string(t["frag"]), GL_FRAGMENT_SHADER).shader);
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

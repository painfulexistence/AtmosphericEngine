#pragma once
#include "common.hpp"

struct Shader
{
    Shader(const std::string& path, GLenum type);
    GLuint shader;
};

class ShaderProgram
{
public:
    ShaderProgram();

    ShaderProgram(std::string, std::string);
    
    ShaderProgram(std::vector<Shader>&);
    
    GLint GetAttrib(const char* attrib) {
        return glGetAttribLocation(program, attrib);
    };

    GLint GetUniform(std::string uniform) {
        return glGetUniformLocation(program, uniform.c_str());
    };

    void SetUniform(std::string uniform, const glm::mat4& val)
    {
        glUniformMatrix4fv(GetUniform(uniform), 1, GL_FALSE, &val[0][0]);
    };

    void SetUniform(std::string uniform, const glm::vec3& val)
    {
        glUniform3fv(GetUniform(uniform), 1, &val[0]);
    };

    void SetUniform(std::string uniform, int val)
    {
        glUniform1i(GetUniform(uniform), val);
    };

    void SetUniform(std::string uniform, float val)
    {
        glUniform1f(GetUniform(uniform), val);
    };

    void Activate();

    void Deactivate();

private:
    GLuint program;
};

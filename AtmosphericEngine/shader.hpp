#pragma once
#include "globals.hpp"
#include <optional>

enum ShaderType
{
    VERTEX = GL_VERTEX_SHADER,
    FRAGMENT = GL_FRAGMENT_SHADER,
    TESS_CONTROL = GL_TESS_CONTROL_SHADER,
    TESS_EVALUATION = GL_TESS_EVALUATION_SHADER
};

struct Shader
{
    Shader(const std::string& path, ShaderType type);
    GLuint shader;
};

struct ShaderProgramProps
{
    std::string vert;
    std::string frag;
    std::optional<std::string> tesc = std::nullopt;
    std::optional<std::string> tese = std::nullopt;
};

class ShaderProgram
{
public:
    ShaderProgram() = default;

    ShaderProgram(const ShaderProgramProps& props);

    ShaderProgram(std::string vert, std::string frag, std::optional<std::string> tesc = std::nullopt, std::optional<std::string> tese = std::nullopt);

    ShaderProgram(std::array<Shader, 2>&);

    ShaderProgram(std::array<Shader, 4>&);

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

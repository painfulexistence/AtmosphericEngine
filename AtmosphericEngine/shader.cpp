#include "shader.hpp"
#include "utility/file.hpp"

Shader::Shader(const std::string& path, ShaderType type)
{
    const auto& shaderSrc = File(path).GetContent();
    const char* src = shaderSrc.c_str();
    const int len = shaderSrc.size();

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, &len);

    glCompileShader(shader);
    GLint isCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        GLchar* log = new GLchar[maxLength];
        glGetShaderInfoLog(shader, maxLength, &maxLength, log);

        throw std::runtime_error(fmt::format("Shader error: {}\n", (char*)log));
    }
}

ShaderProgram::ShaderProgram() {}

ShaderProgram::ShaderProgram(std::string vert, std::string frag, std::optional<std::string> tesc, std::optional<std::string> tese) : program(glCreateProgram())
{
    glAttachShader(program, Shader(vert, ShaderType::VERTEX).shader);
    glAttachShader(program, Shader(frag, ShaderType::FRAGMENT).shader);
    if (tesc.has_value() && tese.has_value()) {
        glAttachShader(program, Shader(tesc.value(), ShaderType::TESS_CONTROL).shader);
        glAttachShader(program, Shader(tese.value(), ShaderType::TESS_EVALUATION).shader);
    }
    glLinkProgram(program);
}

ShaderProgram::ShaderProgram(std::array<Shader, 2>& shaders) : program(glCreateProgram())
{
    for (int i = shaders.size() - 1; i >= 0; i--)
    {
        glAttachShader(program, shaders[i].shader);
    }
    glLinkProgram(program);
}

ShaderProgram::ShaderProgram(std::array<Shader, 4>& shaders) : program(glCreateProgram())
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

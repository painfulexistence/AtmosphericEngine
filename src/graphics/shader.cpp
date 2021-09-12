#include "Graphics/shader.hpp"
#include "System/file.hpp"

static void Compile(GLuint shader)
{
    glCompileShader(shader);

    GLint isCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        
        GLchar* log = new GLchar[maxLength];
        glGetShaderInfoLog(shader, maxLength, &maxLength, log);
                
        std::cout << "Shader error:\n" << (char*)log << std::endl;
    }
}

Shader::Shader(const std::string& path, GLenum type)
{
    const auto& shaderSrc = File(path).GetContent();
    const char* src = shaderSrc.c_str();
    const int len = shaderSrc.size();

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, &len);
    Compile(shader);
}
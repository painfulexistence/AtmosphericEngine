#include "shader.hpp"

Shader::Shader(const std::string& filename, GLenum type)
{
    const auto& shaderSrc = File(filename).GetContent();
    const char* src = shaderSrc.c_str();
    const int len = shaderSrc.size();

    _shader = glCreateShader(type);
    glShaderSource(_shader, 1, &src, &len);
}
    
GLuint Shader::Compile()
{
    glCompileShader(_shader);

    GLint isCompiled;
    glGetShaderiv(_shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &maxLength);
        
        GLchar* log = new GLchar[maxLength];
        glGetShaderInfoLog(_shader, maxLength, &maxLength, log);
                
        std::cout << "Shader error:\n" << (char*)log << std::endl;
    }

    return _shader;
}

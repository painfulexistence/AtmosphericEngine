#include "shader.hpp"

static std::string get_file_content(const char* filename) {
    std::ifstream ifs(filename);
    std::string content(
        (std::istreambuf_iterator<char>(ifs)), //start of stream iterator
        (std::istreambuf_iterator<char>()) //end of stream iterator
    );
    return content;
}

static void load_shader(GLuint shader, const std::string &shader_string) {
    const char* shader_src = shader_string.c_str();
    const int shader_len = shader_string.size();
    glShaderSource(shader, 1, &shader_src, &shader_len);
}

Shader::Shader(const char* filename, GLenum type)
{
    _shader = glCreateShader(type);
    load_shader(_shader, get_file_content(filename));
}
    
Shader::~Shader() { }

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
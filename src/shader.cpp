#include "shader.hpp"

std::string get_file_content(const char* filename) {
    std::ifstream ifs(filename);
    std::string content(
        (std::istreambuf_iterator<char>(ifs)), //start of stream iterator
        (std::istreambuf_iterator<char>()) //end of stream iterator
    );
    return content;
}

void load_shader(GLuint shader, const std::string &shader_string) {
    const char* shader_src = shader_string.c_str();
    const int shader_len = shader_string.size();
    glShaderSource(shader, 1, &shader_src, &shader_len);
}

void Shader::Load() {
    program = glCreateProgram();
    
    shaders[SHADER_V] = glCreateShader(GL_VERTEX_SHADER);
    load_shader(shaders[SHADER_V], get_file_content("shaders/simple.vert"));
    glCompileShader(shaders[SHADER_V]);

    shaders[SHADER_F] = glCreateShader(GL_FRAGMENT_SHADER);
    load_shader(shaders[SHADER_F], get_file_content("shaders/simple.frag"));
    glCompileShader(shaders[SHADER_F]);

    shaders[SHADER_2D_V] = glCreateShader(GL_VERTEX_SHADER);
    load_shader(shaders[SHADER_2D_V], get_file_content("shaders/simple2D.vert"));
    glCompileShader(shaders[SHADER_2D_V]);

    shaders[SHADER_2D_F] = glCreateShader(GL_FRAGMENT_SHADER);
    load_shader(shaders[SHADER_2D_F], get_file_content("shaders/simple2D.frag"));
    glCompileShader(shaders[SHADER_2D_F]);
}

void Shader::Attach(int shader_id) {
    glAttachShader(program, shaders[shader_id]);
}

void Shader::Detach(int shader_id) {
    glAttachShader(program, shaders[shader_id]);
}

void Shader::Activate() {
    glLinkProgram(program);
    glUseProgram(program);
}

void Shader::Deactivate() {
    glUseProgram(0);
}

GLint Shader::GetAttrib(const char* attrib) {
    return glGetAttribLocation(program, attrib);
}

GLint Shader::GetUniform(const char* uniform) {
    return glGetUniformLocation(program, uniform);
}
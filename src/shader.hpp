#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <fstream>
#include <string>

#define NUM_SHADERS 4

#define SHADER_V 0
#define SHADER_F 1
#define SHADER_2D_V 2
#define SHADER_2D_F 3


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
    #ifdef DEBUG
        for (int i = 0; i < shader_len; i++) {
            printf("%c", shader_src[i]);
        }
    #endif
    glShaderSource(shader, 1, &shader_src, &shader_len);
}

class Shader {
    public:
        Shader() {}
        ~Shader() {}

        void load() {
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

        void attach(int shader_id) {
            glAttachShader(program, shaders[shader_id]);
        }

        void detach(int shader_id) {
            glAttachShader(program, shaders[shader_id]);
        }

        void activate() {
            glLinkProgram(program);
            glUseProgram(program);
        }

        void deactivate() {
            glUseProgram(0);
        }

        GLint getAttribLocation(const char* attrib) {
            return glGetAttribLocation(program, attrib);
        }

        GLint getUniformLocation(const char* uniform) {
            return glGetUniformLocation(program, uniform);
        }

    private:
        GLuint program;
        GLuint shaders[NUM_SHADERS];
};

#endif
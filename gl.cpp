#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include <thread>
#include <iostream>
#include "stb_image.h"

const float SCREEN_W = 800.0f;
const float SCREEN_H = 600.0f;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_W, SCREEN_H, "OpenGLExp", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();

    float cube_vertices[] = {
      -1.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, -1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f, 1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, -1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f
    };
    float color_buffer[] = {
      -1.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, -1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f, 1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, -1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f
    };
    float uv_buffer[] = {
      0.25f, 0.67f,
      0.25f, 1.0f,
      0.5f, 0.67f,
      0.5f, 1.0f,
      1.0f, -1.0f,
      -1.0f, -1.0f,
      1.0f, -1.0f,
      -1.0f, 1.0f,
      -1.0f, -1.0f,
      1.0f, -1.0f,
      1.0f, 1.0f,
      1.0f, -1.0f,
      -1.0f, 1.0f,
      -1.0f, -1.0f
    };
    float triforce_vertices[] = {
        //x, y, z, r, g, b
        0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.0f, 0.0f, 0.75f, 0.75f, 0.0f,
        -0.5f, 0.0f, 0.0f, 0.75f, 0.75f, 0.0f,
        0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f
    };
    float money_bro_vertices[] = {
        //hair
        -0.25f, 1.0f, 0.1f, 0.1f, 0.1f,
        0.25f, 1.0f, 0.1f, 0.1f, 0.1f,
        -0.125f, 0.875f, 0.1f, 0.1f, 0.1f,
        0.125f, 0.875f, 0.1f, 0.1f, 0.1f,
        0.375f, 0.875f, 0.1f, 0.1f, 0.1f,
        -0.375f, 0.75f, 0.1f, 0.1f, 0.1f,
        0.0f, 0.75f, 0.1f, 0.1f, 0.1f,
        0.25f, 0.75f, 0.1f, 0.1f, 0.1f,
        0.375f, 0.75f, 0.1f, 0.1f, 0.1f,
        -0.3125f, 0.5625f, 0.1f, 0.1f, 0.1f,
        0.25f, 0.5f, 0.1f, 0.1f, 0.1f,
        //face
        -0.125f, 0.875f, 1.0f, 1.0f, 0.8f,
        0.125f, 0.875f, 1.0f, 1.0f, 0.8f,
        0.0f, 0.75f, 1.0f, 1.0f, 0.8f,
        0.25f, 0.75f, 1.0f, 1.0f, 0.8f,
        -0.3125f, 0.5625f, 1.0f, 1.0f, 0.8f,
        0.25f, 0.5f, 1.0f, 1.0f, 0.8f,
        -0.25f, 0.25f, 1.0f, 1.0f, 0.8f,
        0.125f, 0.25f, 1.0f, 1.0f, 0.8f,
        -0.125f, 0.1875f, 1.0f, 1.0f, 0.8f,
        //neck
        -0.25f, 0.25f, 0.3f, 0.3f, 0.1f,
        0.125f, 0.25f, 0.3f, 0.3f, 0.1f,
        -0.125f, 0.1875f, 0.3f, 0.3f, 0.1f,
        -0.25f, -0.0625f, 1.0f, 1.0f, 0.8f,
        0.125f, -0.0625f, 1.0f, 1.0f, 0.8f,
        -0.125f, -0.1875f, 1.0f, 1.0f, 0.8f,
        //left side
        -0.25f, 0.1875f, 1.0f, 0.0f, 0.0f,
        -0.625f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.25f, -0.0625f, 1.0f, 0.0f, 0.0f,
        -0.875f, -0.875f, 1.0f, 0.0f, 0.0f,
        -0.75f, -1.0f, 1.0f, 0.0f, 0.0f,
        -0.375f, -1.0f, 1.0f, 0.0f, 0.0f,
        //tatoo
        -0.25f, -0.0625f, 0.5f, 1.0f, 0.5f,
        -0.375f, -1.0f, 0.5f, 1.0f, 0.5f,
        -0.125f, -0.1875f, 0.5f, 1.0f, 0.5f,
        0.125f, -1.0f, 0.5f, 1.0f, 0.5f,
        0.0625f, -0.625f, 0.5f, 1.0f, 0.5f,
        0.125f, -0.0625f, 0.5f, 1.0f, 0.5f,
        //right side
        0.125f, -1.0f, 1.0f, 0.0f, 0.0f,
        0.0625f, -0.625f, 1.0f, 0.0f, 0.0f,
        0.125f, -0.0625f, 1.0f, 0.0f, 0.0f,
        0.125f, 0.25f, 1.0f, 0.0f, 0.0f,
        0.375f, 0.0625f, 1.0f, 0.0f, 0.0f,
        0.625f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, -0.5f, 1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        //
        1.0f, 1.0f, 0.5f, 0.5f, 1.0f,
        1.0f, -1.0f, 0.5f, 0.5f, 1.0f,
        -1.0f, -1.0f, 0.5f, 0.5f, 1.0f,
        -1.0f, 1.0f, 0.5f, 0.5f, 1.0f,
    };
    GLuint elements[] = {
        46, 47, 48,
        46, 48, 49,
        //
        0, 1, 6,
        0, 2, 5,
        2, 5, 9,
        1, 3, 7,
        1, 4, 7,
        4, 7, 8,
        7, 8, 10,
        //
        11, 13, 15,
        12, 13, 14,
        13, 15, 17,
        13, 17, 19,
        13, 18, 19,
        13, 16, 18,
        13, 14, 16,
        //
        20, 22, 23,
        22, 23, 25,
        22, 24, 25,
        21, 22, 24,
        //
        26, 27, 28,
        27, 28, 29,
        28, 29, 30,
        28, 30, 31,
        //
        32, 33, 34,
        33, 34, 35,
        34, 35, 36,
        34, 36, 37,
        //
        38, 39, 40,
        38, 40, 41,
        38, 41, 42,
        38, 42, 43,
        38, 43, 44,
        38, 44, 45
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);//Make vbo active
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    const char* vertexSrc = R"glsl(
        #version 330

        in vec3 position;
        in vec3 color;
        in vec2 uv;
        out vec2 raw_uv;
        out vec4 raw_color;

        uniform mat4 mvp;

        void main()
        {
            raw_uv = uv;
            raw_color = vec4(color, 1.0);
            gl_Position = mvp * vec4(position, 1.0);
        }
    )glsl";//Read v shader as raw string
    const char* fragmentSrc = R"glsl(
        #version 330

        in vec2 raw_uv;
        in vec4 raw_color;
        out vec4 outColor;

        uniform sampler2D tex;

        void main()
        {
            outColor = texture(tex, raw_uv);
        }
    )glsl";//Read f shader as raw string
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    GLint colorAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colorAttrib);
    GLint uvAttrib = glGetAttribLocation(shaderProgram, "uv");
    glEnableVertexAttribArray(uvAttrib);
    GLint mvpUniform = glGetUniformLocation(shaderProgram, "mvp");

    GLuint texture;
    glGenTextures(1, &texture);

    int width, height, nChannels;
    unsigned char *data = stbi_load("top.bmp", &width, &height, &nChannels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    stbi_image_free(data);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

        glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer), color_buffer, GL_STATIC_DRAW);
        glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer), uv_buffer, GL_STATIC_DRAW);
        glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);

        glm::mat4 model = glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(100, 100, 100)), (float)(glfwGetTime() * glm::radians(45.0f)), glm::vec3(0, 0, 1));
        glm::mat4 view = glm::lookAt(glm::vec3(8, 8, 8), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), SCREEN_W / SCREEN_H, 0.1f, 200.0f);
        glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, &mvp[0][0]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
        glfwSwapBuffers(window);

        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }
    glfwTerminate();
}

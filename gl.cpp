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
#include "terrain.hpp"
#include "cube.hpp"

const float SCREEN_W = 800.0f;
const float SCREEN_H = 600.0f;

int main() {
    //Initialization code
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

    //Define shader program
    const char* vertexSrc = R"glsl(
        #version 330

        in vec3 position;
        in vec3 color;
        in vec2 uv;
        out vec2 tex_uv;
        out vec3 tex_hue;
        uniform mat4 mvp;

        void main()
        {
            tex_uv = uv;
            tex_hue = color;
            gl_Position = mvp * vec4(position, 1.0);
        }
    )glsl";
    const char* fragmentSrc = R"glsl(
        #version 330

        in vec2 tex_uv;
        in vec3 tex_hue;
        out vec4 outColor;

        uniform sampler2D tex_one;

        void main()
        {
            outColor = texture(tex_one, tex_uv) * vec4(tex_hue, 1.0);
        }
    )glsl";
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

    //Shader parameters
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    GLint colorAttrib = glGetAttribLocation(shaderProgram, "color");
    GLint uvAttrib = glGetAttribLocation(shaderProgram, "uv");
    GLint mvpUniform = glGetUniformLocation(shaderProgram, "mvp");
    GLint tex_one = glGetUniformLocation(shaderProgram, "tex_one");

    //VAO, VBO, EBO
    GLuint vao1, vbo1, ebo1;
    glGenVertexArrays(1, &vao1);
    glGenBuffers(1, &vbo1);
    glGenBuffers(1, &ebo1);

    GLuint vao2, vbo2, ebo2;
    glGenVertexArrays(1, &vao2);
    glGenBuffers(1, &vbo2);
    glGenBuffers(1, &ebo2);

    glBindVertexArray(vao1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo1);
    glEnableVertexAttribArray(posAttrib);
    glEnableVertexAttribArray(colorAttrib);
    glEnableVertexAttribArray(uvAttrib);

    glBindVertexArray(vao2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo2);
    glEnableVertexAttribArray(posAttrib);
    glEnableVertexAttribArray(colorAttrib);
    glEnableVertexAttribArray(uvAttrib);

    //Load textures
    std::string textures[7] = {
      "res/grass.png",
      "res/brick.jpg",
      "res/beach.png",
      "res/skybox/bottom.bmp",
      "res/skybox/top.bmp",
      "res/skybox/front.bmp",
      "res/starnight.jpg"
    };
    for (int i = 0; i < 7; i++) {
      GLuint tex;
      glGenTextures(1, &tex);
      int width, height, nChannels;
      unsigned char *data = stbi_load(textures[i].c_str(), &width, &height, &nChannels, 0);
      if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
      } else {
        std::cout << "Failed to load image!" << std::endl;
      }
      stbi_image_free(data);
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, tex);
    }

    //Create object
    float heightmap[100] = {
      1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
      2.0f, 3.0f, 6.0f, 8.0f, 5.0f, 3.0f, 5.0f, 8.0f, 10.0f, 6.0f,
      5.0f, 7.0f, 5.0f, 18.0f, 30.0f, 50.0f, 7.0f, 5.0f, 18.0f, 10.0f,
      20.0f, 23.0f, 6.0f, 20.0f, 50.0f, 2.0f, 3.0f, 6.0f, 8.0f, 5.0f,
      30.0f, 5.0f, 8.0f, 30.0f, 60.0f, 80.0f, 100.0f, 6.0f, 8.0f, 5.0f,
      52.0f, 7.0f, 5.0f, 18.0f, 10.0f, 50.0f, 100.0f, 6.0f, 8.0f, 5.0f,
      75.0f, 52.0f, 30.0f, 12.0f, 10.0f, 30.0f, 2.0f, -3.0f, 0.0f, 5.0f,
      20.0f, 30.0f, 6.0f, 8.0f, 5.0f, 3.0f, -5.0f, -18.0f, -10.0f, 1.0f,
      50.0f, 70.0f, 5.0f, 18.0f, 10.0f, 5.0f, -17.0f, -5.0f, 18.0f, 20.0f,
      20.0f, 30.0f, 6.0f, 8.0f, 5.0f, 1.0f, 2.0f, 30.0f, 40.0f, 50.0f
    };
    Terrain* t = new Terrain(256, 10, heightmap);
    Cube* c = new Cube(2000);

    //Put camera
    glm::vec3 position = glm::vec3(-100, 150, 0);
    float hAngle = 0.0f;
    float vAngle = -1.0f;
    float speed = 20.0f;
    float aSpeed = 3.14f/2.0f;
    float maxVAngle = 3.14f/2.0f;
    float minVAngle = -3.14f/2.0f;

    //Set timer
    double pastTime = glfwGetTime();
    while(!glfwWindowShouldClose(window))
    {
        //clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //set up renderer
        glUseProgram(shaderProgram);
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(0xFFFF);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        //set up mvp
        glm::vec3 direction = glm::vec3(glm::cos(hAngle) * glm::cos(vAngle), glm::sin(vAngle), glm::sin(hAngle) * glm::cos(vAngle));
        glm::vec3 up = glm::vec3(0, 1, 0);
        glm::vec3 right = glm::cross(direction, up);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), SCREEN_W / SCREEN_H, 0.1f, 5000.0f);
        glm::mat4 view = glm::lookAt(position, position + direction, up);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = projection * view * model;

        //Render terrain
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        model = glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)), (float)(glfwGetTime() * glm::radians(0.0f)), up);
        mvp = projection * view * model;
        glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, &mvp[0][0]);
        glUniform1i(tex_one, 1);

        glBindVertexArray(vao1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo1);
        glBufferData(GL_ARRAY_BUFFER, t->countVertices()*sizeof(float), t->getVertexArray(), GL_STATIC_DRAW);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
        glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
        glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, t->countElements()*sizeof(GLushort), t->getElementArray(), GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, t->countElements(), GL_UNSIGNED_SHORT, 0);

        //Render cube model
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        model = glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(2000.0f, 2000.0f, 2000.0f)), (float)(glfwGetTime() * glm::radians(3.0f)), up);
        mvp = projection * view * model;
        glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, &mvp[0][0]);
        glUniform1i(tex_one, 5);

        glBindVertexArray(vao2);
        glBindBuffer(GL_ARRAY_BUFFER, vbo2);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo2);
        glBufferData(GL_ARRAY_BUFFER, c->countVertices()*sizeof(float), c->getVertexArray(), GL_STATIC_DRAW);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
        glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
        glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, c->countElements()*sizeof(GLushort), c->getElementArray(), GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLES, c->countElements(), GL_UNSIGNED_SHORT, 0); //glDrawArrays(GL_TRIANGLES, 0, 24);

        //Finish rendering
        glfwSwapBuffers(window);

        //Process events and update
        glfwPollEvents();
        double currentTime = glfwGetTime();
        float dt = float(currentTime - pastTime);
        pastTime = currentTime;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            position += direction * dt * speed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            position -= direction * dt * speed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            position += right * dt * speed;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            position -= right * dt * speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            hAngle += dt * aSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            hAngle -= dt * aSpeed;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && vAngle + dt * aSpeed < maxVAngle)
            vAngle += dt * aSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && vAngle - dt * aSpeed > minVAngle)
            vAngle -= dt * aSpeed;
    }
    glfwTerminate();
}

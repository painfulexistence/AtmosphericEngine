#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../common.hpp"

class Framework
{
    GLFWwindow* window = nullptr;
    GLuint shadowFramebuffer;
    
    std::vector<GLuint> textures;
    GLuint normalMap;
    GLuint diffuseMap;
    GLuint specularMap;
    GLuint shadowMap;
    GLuint metallicMap;

public:
    void Init();
    void AddTextures(const std::vector<std::string>&);
    void SwapBuffers();
    void PollEvents();
    void CloseWindow();
    void Terminate();
    void CheckErrors();

    std::vector<GLuint> GetTextures()
    {
        return textures;
    };

    GLuint GetShadowMap()
    {
        return shadowMap;
    };

    GLuint GetShadowFramebuffer()
    {
        return shadowFramebuffer;
    };

    double GetTime()
    {
        return glfwGetTime();
    };
    
    glm::vec2 GetCursorPosition() 
    {
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        return glm::vec2(xPos, yPos);
    };
    
    glm::vec2 GetCursorUV()
    {
        glm::vec2 cursorPos = GetCursorPosition();
        return glm::vec2(cursorPos.x / SCREEN_W, cursorPos.y / SCREEN_H);
    };

    bool IsWindowOpen()
    {
        return glfwWindowShouldClose(window);
    };

    bool IsKeyDown(int key)
    {
        return (glfwGetKey(window, key) == GLFW_PRESS);
    };
};

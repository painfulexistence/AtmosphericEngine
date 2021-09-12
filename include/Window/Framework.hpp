#pragma once
#include "../common.hpp"

class Framework
{
public:
    GLFWwindow* window = nullptr;

    Framework();

    ~Framework();

    void SwapBuffers();
    
    void PollEvents();
    
    void CloseWindow();
    
    double GetTime() { return glfwGetTime(); }
    
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

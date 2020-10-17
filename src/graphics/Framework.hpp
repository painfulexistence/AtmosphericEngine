#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../common.hpp"


class Framework
{
private:
    GLFWwindow* window = nullptr;
    bool shouldClose = false;

public:
    Framework();

    ~Framework();

    void Init();

    void Swap();

    void Poll();

    bool KeyDown(int);

    glm::vec2 CursorPosition();

    glm::vec2 CursorUV();

    double GetTime();

    bool ShouldCloseWindow();

    void CloseWindow();

    void Terminate();
};

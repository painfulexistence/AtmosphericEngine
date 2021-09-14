#pragma once
#include "common.hpp"

class Framework
{
public:
    Framework();

    ~Framework();

    bool Run();
    
    void Draw();

    float GetTime();

    float GetFrameTime();
    
    glm::vec2 GetCursor();
    
    bool IsKeyDown(int key);

    bool IsKeyUp(int key);

private:
    GLFWwindow* _window = nullptr;
    double _time;
};

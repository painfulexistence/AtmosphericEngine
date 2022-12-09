#pragma once
#include "Globals.hpp"

const int KEY_UP = GLFW_KEY_UP;
const int KEY_RIGHT = GLFW_KEY_RIGHT;
const int KEY_LEFT = GLFW_KEY_LEFT;
const int KEY_DOWN = GLFW_KEY_DOWN;
const int KEY_Q = GLFW_KEY_Q;
const int KEY_W = GLFW_KEY_W;
const int KEY_E = GLFW_KEY_E;
const int KEY_R = GLFW_KEY_R;
const int KEY_A = GLFW_KEY_A;
const int KEY_S = GLFW_KEY_S;
const int KEY_D = GLFW_KEY_D;
const int KEY_F = GLFW_KEY_F;
const int KEY_Z = GLFW_KEY_Z;
const int KEY_X = GLFW_KEY_X;
const int KEY_C = GLFW_KEY_C;
const int KEY_V = GLFW_KEY_V;
const int KEY_ESCAPE = GLFW_KEY_ESCAPE;
const int KEY_ENTER = GLFW_KEY_ENTER;
const int KEY_SPACE = GLFW_KEY_SPACE;

class Window;

class Application
{
public:
    static std::optional<Application> instance;

    Application();

    ~Application();

    void Tick();
    
    uint64_t GetClock();

    float GetTime();

    Window* GetActiveWindow();
    
private:
    uint64_t _clock = 0;
    Window* _activeWindow = nullptr;  
};

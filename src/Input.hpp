#pragma once
#include "Globals.hpp"
#include "Window.hpp"
#include "Server.hpp"

class Input : public Server
{
public:
    Input();
    
    ~Input();
    
    bool GetKeyDown(int key);
    
    bool GetKeyUp(int key);
    
    glm::vec2 GetMousePosition();
    
private:
    std::vector<int> keys
    {
        KEY_UP,
        KEY_RIGHT,
        KEY_LEFT,
        KEY_DOWN,
        KEY_Q,
        KEY_W,
        KEY_E,
        KEY_R,
        KEY_A,
        KEY_S,
        KEY_D,
        KEY_F,
        KEY_Z,
        KEY_X,
        KEY_C,
        KEY_V,
        KEY_ESCAPE,
        KEY_ENTER,
        KEY_SPACE
    };
};
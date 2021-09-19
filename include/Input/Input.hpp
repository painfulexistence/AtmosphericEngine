#pragma once
#include "Globals.hpp"
#include "Framework.hpp"

class Input : public Server
{
public:
    Input();
    
    ~Input();
    
    bool GetKeyDown(int key);
    
    bool GetKeyUp(int key);
    
    glm::vec2 GetMousePosition();
    
    glm::vec2 GetMouseUV();

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
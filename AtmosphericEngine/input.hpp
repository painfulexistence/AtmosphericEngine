#pragma once
#include "globals.hpp"
#include "window.hpp"
#include "server.hpp"
#include <GLFW/glfw3.h>

const int KEY_UP = GLFW_KEY_UP;
const int KEY_RIGHT = GLFW_KEY_RIGHT;
const int KEY_LEFT = GLFW_KEY_LEFT;
const int KEY_DOWN = GLFW_KEY_DOWN;
const int KEY_Q = GLFW_KEY_Q;
const int KEY_W = GLFW_KEY_W;
const int KEY_E = GLFW_KEY_E;
const int KEY_R = GLFW_KEY_R;
const int KEY_T = GLFW_KEY_T;
const int KEY_Y = GLFW_KEY_Y;
const int KEY_U = GLFW_KEY_U;
const int KEY_I = GLFW_KEY_I;
const int KEY_O = GLFW_KEY_O;
const int KEY_P = GLFW_KEY_P;
const int KEY_A = GLFW_KEY_A;
const int KEY_S = GLFW_KEY_S;
const int KEY_D = GLFW_KEY_D;
const int KEY_F = GLFW_KEY_F;
const int KEY_G = GLFW_KEY_G;
const int KEY_H = GLFW_KEY_H;
const int KEY_J = GLFW_KEY_J;
const int KEY_K = GLFW_KEY_K;
const int KEY_L = GLFW_KEY_L;
const int KEY_Z = GLFW_KEY_Z;
const int KEY_X = GLFW_KEY_X;
const int KEY_C = GLFW_KEY_C;
const int KEY_V = GLFW_KEY_V;
const int KEY_B = GLFW_KEY_B;
const int KEY_N = GLFW_KEY_N;
const int KEY_M = GLFW_KEY_M;
const int KEY_ESCAPE = GLFW_KEY_ESCAPE;
const int KEY_ENTER = GLFW_KEY_ENTER;
const int KEY_SPACE = GLFW_KEY_SPACE;

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
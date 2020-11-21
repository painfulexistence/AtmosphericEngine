#pragma once
#include "common.hpp"
#include "physics/BulletMain.h"
#include "graphics/Framework.hpp"
#include "engine/Engine.hpp"
#include "scripting/lua.hpp"
//#include <entt/entity/registry.hpp>

//ImGui
const ImVec4 clearColor = ImVec4(0.15f, 0.183f, 0.2f, 1.0f);

//Key mapping
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

class Game
{
    Framework& framework;
    PhysicsWorld world;
    Scene scene;
    Camera camera;
    Light mainLight;
    std::vector<Light> auxLights = {};
    Program colorProgram;
    Program depthProgram;

    Lua lua;
    std::list<Entity>& entities;

    void CreateMaze();
        
    void HandleInput();
        
    void RenderGUI(float);
    
public:
    Game(Framework&);
        
    void Run();

    void Update(float dt, float time);

    void Render(float dt);
};

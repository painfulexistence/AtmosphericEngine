#pragma once
#include "../common.hpp"
#include "../physics/BulletMain.h"
#include "../graphics/Framework.hpp"
#include "Engine.hpp"

//ImGui
const ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
    struct GameState {
        double timeAccumulator;
        glm::vec3 lightColor;
        bool isLightFlashing;
        glm::vec3 winPosition;
    };
    std::shared_ptr<Framework> _framework;
    std::shared_ptr<btDiscreteDynamicsWorld> _world;
    std::shared_ptr<Program> _program;
    Scene _scene;
    GameState _state;

    Camera* camera;
    Light* light;
    
public:
    Game(const std::shared_ptr<Framework>&);
    
    ~Game();

    void Init();

    void Start();

    void LoadResources();
    
    void CreateMaze(const std::vector<std::vector<bool>>&, std::vector<std::shared_ptr<Geometry>>&);

    void Update(float);

    void HandleInput();

    void Render(float);

    void RenderGUI(float);
};

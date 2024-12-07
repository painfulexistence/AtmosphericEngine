#pragma once
#include "globals.hpp"
#include "imgui.h"

class Application;

class Server {
public:
    Server();
    ~Server();

    virtual void Init(Application* app);
    virtual void Process(float dt);
    virtual void DrawImGui(float dt);

protected:
    Application* _app;
    float _timeStep;
    int _maxNumSteps;
    float _paused = false;
};
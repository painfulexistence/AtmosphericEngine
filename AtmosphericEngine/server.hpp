#pragma once
#include "globals.hpp"

class Application;

class Server
{
public:
    Server();

    ~Server();

    void Init(Application* app);

    virtual void Process(float dt);

protected:
    Application* _app;
    float _timeStep;
    int _maxNumSteps;
    float _paused = false;
};
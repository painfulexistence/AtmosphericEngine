#pragma once
#include "globals.hpp"
#include "messagable.hpp"
#include "message.hpp"
#include "message_bus.hpp"

class Application;

class Server : public Messagable
{
public:
    Server();

    ~Server();

    void Init(MessageBus* mb, Application* app);

    virtual void Process(float dt);

    virtual void OnMessage(Message msg);

protected:
    Application* _app;
    float _timeStep;
    int _maxNumSteps;
    float _paused = false;
};
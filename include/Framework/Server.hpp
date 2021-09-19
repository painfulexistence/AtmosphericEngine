#pragma once
#include "Globals.hpp"
#include "Messaging/Messagable.hpp"
#include "Messaging/Message.hpp"
#include "Messaging/MessageBus.hpp"
#include "Framework/Application.hpp"

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
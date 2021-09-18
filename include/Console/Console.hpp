#pragma once
#include "Globals.hpp"
#include "Messaging.hpp"
#include "Framework.hpp"

class Console : public Messagable
{
public:
    Console();
    ~Console();
    void Init(MessageBus* mb, Application* app);
    void HandleMessage(Message msg) override;
private:
    Application* _app;
};
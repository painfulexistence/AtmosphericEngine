#pragma once
#include "Globals.hpp"
#include "Messaging.hpp"
#include "Framework.hpp"

class Script : public Messagable
{
public:
    Script();
    ~Script();
    void Init(MessageBus* mb, Application* app);
    void HandleMessage(Message msg) override;

private:
    Application* _app;
};
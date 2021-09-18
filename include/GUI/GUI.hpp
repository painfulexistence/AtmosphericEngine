#pragma once
#include "Globals.hpp"
#include "Messaging.hpp"
#include "Framework.hpp"

class GUIState;

class GUI : public Messagable
{
public:
    GUI();
    ~GUI();
    void Init(MessageBus* mb, Application* app);
    void HandleMessage(Message msg) override;
    void Render(float dt);
private:
    Application* _app;
    GUIState* _state;
};
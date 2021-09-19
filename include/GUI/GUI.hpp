#pragma once
#include "Globals.hpp"
#include "Framework.hpp"

class GUIState;

class GUI : public Server
{
public:
    GUI();
    ~GUI();
    void Init(MessageBus* mb, Application* app);
    void Process(float dt) override;
    void OnMessage(Message msg) override;
    void Render(float dt);
private:
    GUIState* _state;
};
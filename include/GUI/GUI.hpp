#pragma once
#include "common.hpp"
#include "Messaging.hpp"
#include "Framework.hpp"

class GUIState;

class GUI : public Messagable
{
public:
    GUI();
    ~GUI();
    void Init(MessageBus* mb, Framework* fw);
    void HandleMessage(Message msg) override;
    void Render();
private:
    Framework* _fw;
    GUIState* _state;
};
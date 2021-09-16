#pragma once
#include "common.hpp"
#include "Messaging.hpp"
#include "Framework.hpp"

class Script : public Messagable
{
public:
    Script();
    ~Script();
    void Init(MessageBus* mb, Framework* fw);
    void HandleMessage(Message msg) override;

private:
    Framework* _fw;
};
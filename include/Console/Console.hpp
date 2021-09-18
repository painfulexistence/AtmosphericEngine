#pragma once
#include "Globals.hpp"
#include "Messaging/Messagable.hpp"
#include "Messaging/Message.hpp"

class Framework;
class MessageBus;
class Console : public Messagable
{
public:
    Console();
    ~Console();
    void Init(MessageBus* mb, Framework* fw);
    void HandleMessage(Message msg) override;
private:
    Framework* _fw;
};
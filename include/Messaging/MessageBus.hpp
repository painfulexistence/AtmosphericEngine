#pragma once
#include "common.hpp"
#include "Messaging/Message.hpp"
#include "Messaging/Messagable.hpp"

class MessageBus
{
private:
    std::list<Messagable> messagables;
public:
    MessageBus();
    int Register(Messagable messagable);
    void Unregister();
    void PostMessage(Message msg);
    void PostImmediateMessage(Message msg);
};
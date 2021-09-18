#pragma once
#include "Globals.hpp"
#include "Messaging/Message.hpp"

class MessageBus;
class Messagable
{
public:
    Messagable();
    void ConnectBus(MessageBus* mb);
    void ReceiveMessage(Message msg);
protected:
    MessageBus* messageBus;
    void SendMessage(Message msg);
    void SendImmediateMessage(Message msg);
    virtual void HandleMessage(Message msg);
};
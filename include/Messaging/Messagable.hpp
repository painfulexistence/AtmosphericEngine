#pragma once
#include "common.hpp"
#include "Messaging/Message.hpp"

class Messagable
{
private:
    std::queue<Message> messageBuffer;
    void HandleMessage(Message msg);
public:
    Messagable();
    void SendMessage(Message msg);
    void SendImmediateMessage(Message msg);
};
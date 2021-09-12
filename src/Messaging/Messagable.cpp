#include "Messaging/Messagable.hpp"

Messagable::Messagable()
{
    messageBuffer = std::queue<Message>();
}

void Messagable::SendMessage(Message msg)
{
    messageBuffer.push(msg);
}

void Messagable::SendImmediateMessage(Message msg)
{
    HandleMessage(msg);
}

void Messagable::HandleMessage(Message msg)
{
    switch(msg.type)
    {
        default:
            break;
    }
}
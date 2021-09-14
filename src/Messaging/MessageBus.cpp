#include "Messaging/MessageBus.hpp"
#include "Messaging/Messagable.hpp"
#include "Runtime.hpp"

MessageBus::MessageBus()
{

}

MessageBus::~MessageBus()
{

}

void MessageBus::Supervise(Runtime* supervisor)
{
    this->_supervisors.push_back(supervisor);
}

int MessageBus::Register(Messagable* receiver)
{
    int id = _receivers.size();
    this->_receivers.push_back(receiver);
    return id;
}

void MessageBus::PostMessage(Message msg)
{
    if (msg.type == MessageType::ON_QUIT)
    {
        for (auto rt : _supervisors)
        {
            rt->Quit();
        }
        return;
    }
    _messages.push(msg);
}

void MessageBus::PostImmediateMessage(Message msg)
{
    if (msg.type == MessageType::ON_QUIT)
    {
        for (auto rt : _supervisors)
        {
            rt->Quit();
        }
        return;
    }
    for (auto m : _receivers)
    {
        m->ReceiveMessage(msg);
    }
}

void MessageBus::Notify()
{
    while (!_messages.empty())
    {
        auto msg = _messages.front();
        for (auto m : _receivers)
        {
            m->ReceiveMessage(msg);
        }
        OnMessageSent(msg);
        
        _messages.pop();
    }
}

void MessageBus::OnMessageSent(Message msg)
{
    PostImmediateMessage(Message(MessageType::ON_MESSAGE_SENT));
}
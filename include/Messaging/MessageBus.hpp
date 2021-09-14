#pragma once
#include "common.hpp"
#include "Messaging/Message.hpp"

class Runtime;
class Messagable;
class MessageBus
{
public:
    MessageBus();
    ~MessageBus();
    void Supervise(Runtime* supervisor);
    int Register(Messagable* receiver);
    void PostMessage(Message msg);
    void PostImmediateMessage(Message msg);
    void Notify();
private:
    std::list<Runtime*> _supervisors;
    std::list<Messagable*> _receivers;
    std::queue<Message> _messages;
    void OnMessageSent(Message msg);
};
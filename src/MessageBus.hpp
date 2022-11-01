#pragma once
#include "Globals.hpp"
#include "Message.hpp"

class Runtime;
class Messagable;
class MessageBus
{
public:
    MessageBus(Runtime* supervisor);
    ~MessageBus();
    int Register(Messagable* receiver);
    void PostMessage(Message msg);
    void PostImmediateMessage(Message msg);
    void Process();
private:
    const int MAX_PROCESSING_NUM_MSGS = 20;
    Runtime* _supervisor;
    std::list<Messagable*> _receivers;
    std::queue<Message> _messages;
    void OnMessageSent(Message msg);
};
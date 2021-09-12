#include "Messaging/MessageBus.hpp"
#include "Messaging/Messagable.hpp"

MessageBus::MessageBus()
{
    messagables = std::list<Messagable>();
}

int MessageBus::Register(Messagable messagable)
{
    int id = messagables.size();
    messagables.push_back(messagable);
    return id;
}

void MessageBus::Unregister()
{

}

void MessageBus::PostMessage(Message msg)
{
    for(auto m : messagables)
    {
        m.SendMessage(msg);
    }
}

void MessageBus::PostImmediateMessage(Message msg)
{
    for(auto m : messagables)
    {
        m.SendImmediateMessage(msg);
    }
}
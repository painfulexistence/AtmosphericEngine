#include "Console/Console.hpp"
#include "Messaging.hpp"
#include "Framework.hpp"

Console::Console()
{

}

Console::~Console()
{

}

void Console::Init(MessageBus* mb, Framework* fw)
{
    ConnectBus(mb);
    this->_fw = fw;
}

void Console::HandleMessage(Message msg)
{
    std::cout << msg.type << std::endl;
}
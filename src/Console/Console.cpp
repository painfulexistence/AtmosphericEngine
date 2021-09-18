#include "Console/Console.hpp"

Console::Console()
{

}

Console::~Console()
{

}

void Console::Init(MessageBus* mb, Application* app)
{
    ConnectBus(mb);
    this->_app = app;
}

void Console::HandleMessage(Message msg)
{

}
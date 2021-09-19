#include "Framework/Server.hpp"

Server::Server()
{

}

Server::~Server()
{

}

void Server::Init(MessageBus* mb, Application* app)
{
    ConnectBus(mb);
    this->_app = app;
}

void Server::Process(float dt)
{
    // Default processing
}

void Server::OnMessage(Message msg)
{
    // Default message handlers
    switch (msg.type)
    {
        default:
            break;
    }
}
#include "Server.hpp"
#include "Application.hpp"

Server::Server()
{

}

Server::~Server()
{

}

void Server::Init(MessageBus* mb, Application* app)
{
    // Note that this method should NOT be simply derived, it should be called in child class
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
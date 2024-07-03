#include "server.hpp"
#include "application.hpp"

Server::Server()
{

}

Server::~Server()
{

}

void Server::Init(Application* app)
{
    this->_app = app;
}

void Server::Process(float dt)
{
    // Default processing
}
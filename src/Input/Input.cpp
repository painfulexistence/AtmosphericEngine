#include "Input/Input.hpp"

Input::Input()
{

}

Input::~Input()
{

}

void Input::Init(MessageBus* mb, Application* app)
{
    ConnectBus(mb);
    this->_app = app;
}

void Input::HandleMessage(Message msg)
{
    switch (msg.type)
    {
        default:
            break;
    }
}

bool Input::GetKeyDown(int key)
{
    bool isDown = this->_app->GetActiveWindow()->GetKeyDown(key);
    if (isDown && key == KEY_ESCAPE)
    {
        messageBus->PostMessage(MessageType::ON_QUIT);
    }
    return isDown;
}

bool Input::GetKeyUp(int key)
{
    return this->_app->GetActiveWindow()->GetKeyUp(key);
}

glm::vec2 Input::GetMousePosition() // In pixel coordinate
{
    return this->_app->GetActiveWindow()->GetMousePosition();
};

glm::vec2 Input::GetMouseUV() // In uv coordinate
{
    glm::vec2 pos = GetMousePosition();
    return glm::vec2(pos.x / SCREEN_W, pos.y / SCREEN_H);
};
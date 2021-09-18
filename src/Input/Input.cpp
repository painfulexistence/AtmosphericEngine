#include "Input/Input.hpp"

Input::Input()
{

}

Input::~Input()
{

}

void Input::Init(MessageBus* mb, Framework* fw)
{
    ConnectBus(mb);
    this->_fw = fw;
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
    bool isDown = this->_fw->GetActiveWindow()->GetKeyDown(key);
    if (isDown && key == KEY_ESCAPE)
    {
        messageBus->PostMessage(MessageType::ON_QUIT);
    }
    return isDown;
}

bool Input::GetKeyUp(int key)
{
    return this->_fw->GetActiveWindow()->GetKeyUp(key);
}

glm::vec2 Input::GetMousePosition() // In pixel coordinate
{
    return this->_fw->GetActiveWindow()->GetMousePosition();
};

glm::vec2 Input::GetMouseUV() // In uv coordinate
{
    glm::vec2 pos = GetMousePosition();
    return glm::vec2(pos.x / SCREEN_W, pos.y / SCREEN_H);
};
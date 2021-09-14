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
    bool isDown = this->_fw->IsKeyDown(key);
    if (isDown && key == KEY_ESCAPE)
    {
        messageBus->PostImmediateMessage(MessageType::ON_QUIT);
    }
    return isDown;
}

bool Input::GetKeyUp(int key)
{
    bool isUp = this->_fw->IsKeyUp(key);
    return isUp;
}

glm::vec2 Input::GetCursorPos()
{
    glm::vec2 cursor = this->_fw->GetCursor();
    return cursor;
};

glm::vec2 Input::GetCursorUV()
{
    glm::vec2 pos = GetCursorPos();
    return glm::vec2(pos.x / SCREEN_W, pos.y / SCREEN_H);
};
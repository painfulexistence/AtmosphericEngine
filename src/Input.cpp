#include "Input.hpp"

Input::Input()
{

}

Input::~Input()
{

}

bool Input::GetKeyDown(int key)
{
    bool isDown = this->_app->GetActiveWindow()->GetKeyDown(key);
    if (isDown && key == KEY_ESCAPE)
    {
        SendMessage(MessageType::ON_QUIT);
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
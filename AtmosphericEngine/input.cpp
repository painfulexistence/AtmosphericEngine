#include "input.hpp"
#include "application.hpp"

Input::Input()
{

}

Input::~Input()
{

}

bool Input::GetKeyDown(int key)
{
    bool isDown = this->_app->GetWindow()->GetKeyDown(key);
    return isDown;
}

bool Input::GetKeyUp(int key)
{
    return this->_app->GetWindow()->GetKeyUp(key);
}

glm::vec2 Input::GetMousePosition() // In pixel coordinate
{
    return this->_app->GetWindow()->GetMousePosition();
};
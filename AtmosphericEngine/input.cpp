#include "input.hpp"
#include "window.hpp"

Input* Input::_instance = nullptr;

Input::Input()
{
    if (_instance != nullptr)
        throw std::runtime_error("Input is already initialized!");

    _instance = this;
}

Input::~Input()
{

}

bool Input::GetKeyDown(int key)
{
    return Window::Get()->GetKeyDown(key);
}

bool Input::GetKeyUp(int key)
{
    return Window::Get()->GetKeyUp(key);
}

glm::vec2 Input::GetMousePosition() // In pixel coordinate
{
    return Window::Get()->GetMousePosition();
};
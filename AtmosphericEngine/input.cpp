#include "input.hpp"
#include "window.hpp"

Input* Input::_instance = nullptr;

Input::Input()
{
    if (_instance != nullptr)
        throw std::runtime_error("Input is already initialized!");

    _instance = this;

    for (int k = static_cast<int>(Key::SPACE); k <= static_cast<int>(Key::ESCAPE); ++k) {
        auto key = static_cast<Key>(k);
        _keyStates[key] = KeyState::UNKNOWN;
    }
}

Input::~Input()
{

}

void Input::Process(float dt)
{
    for (int k = static_cast<int>(Key::SPACE); k <= static_cast<int>(Key::ESCAPE); ++k) {
        auto key = static_cast<Key>(k);
        _keyStates[key] = Window::Get()->GetKeyState(key);
    }
}

bool Input::GetKeyDown(Key key)
{
    return Window::Get()->GetKeyDown(key);
}

bool Input::GetKeyUp(Key key)
{
    return Window::Get()->GetKeyUp(key);
}

bool Input::IsKeyDown(Key key)
{
    return _keyStates[key] == KeyState::PRESSED;
}

bool Input::IsKeyUp(Key key)
{
    return _keyStates[key] == KeyState::RELEASED;
}

glm::vec2 Input::GetMousePosition() // In pixel coordinate
{
    return Window::Get()->GetMousePosition();
};
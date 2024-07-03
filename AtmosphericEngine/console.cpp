#include "console.hpp"

Console* Console::_instance = nullptr;

Console::Console()
{
    if (_instance != nullptr)
        throw std::runtime_error("Console is already initialized!");

    _instance = this;
}

Console::~Console()
{

}
#pragma once
#include "Globals.hpp"

class GameObject;

class Component
{
public:
    GameObject* gameObject = nullptr;

    virtual std::string GetName() const = 0;
};
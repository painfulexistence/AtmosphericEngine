#pragma once
#include "Globals.hpp"

class GameObject;

class Component
{
public:
    GameObject* gameObject = nullptr;

    virtual ~Component() {};

    virtual std::string GetName() const = 0;
};
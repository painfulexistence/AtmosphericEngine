#pragma once
#include "Globals.hpp"

class Entity;
class Component
{
public:
    Entity& entity;

    Component(Entity& entity);

    ~Component();

private:
};
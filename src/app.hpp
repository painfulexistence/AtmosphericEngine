#pragma once
#include "common.hpp"
#include "graphics/Framework.hpp"
#include "game.hpp"

class Application
{
private:
    Framework framework;

public:
    Application();

    void Init();

    void Run();
};

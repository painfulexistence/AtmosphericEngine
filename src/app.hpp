#pragma once
#include "common.hpp"
#include "graphics/Framework.hpp"
#include "engine/game.hpp"


class Application
{
private:
    Framework* framework;
    Game* game;
    double pastTime;
public:
    Application();

    ~Application();

    void Init();

    void Run();

    void MainLoop();

    void Cleanup();
};

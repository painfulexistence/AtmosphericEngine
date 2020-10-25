#pragma once
#include "common.hpp"
#include "graphics/Framework.hpp"
#include "engine/game.hpp"

class Application
{
    std::shared_ptr<Framework> framework;

public:
    Application();
    ~Application();
    void Init();
    void Run();
};

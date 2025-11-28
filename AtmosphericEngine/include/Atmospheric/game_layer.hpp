#pragma once
#include "layer.hpp"
#include <glad/glad.h>

class Application;

class GameLayer : public Layer {
public:
    GameLayer(Application* app);
    ~GameLayer() = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float dt) override;
    void OnRender(float dt) override;

private:
    Application* _app;
};

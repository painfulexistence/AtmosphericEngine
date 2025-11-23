#include "game_layer.hpp"
#include "application.hpp"
#include "camera_component.hpp"
#include "game_object.hpp"
#include "imgui.h"
#include "light_component.hpp"
#include "mesh_component.hpp"
#include "rigidbody_component.hpp"
#include "sprite_component.hpp"
#include "window.hpp"

GameLayer::GameLayer(Application* app) : Layer("GameLayer"), _app(app) {
}

void GameLayer::OnUpdate(float dt) {
    for (auto& entity : _app->GetEntities()) {
        entity->Tick(dt);
    }
}

void GameLayer::OnRender(float dt) {
    // Note that draw calls are asynchronous, which means they return immediately.
    // So the drawing time can only be calculated along with the image presenting.
    _app->GetGraphicsServer()->Render(dt);
    // Nevertheless, glFinish() can force the GPU process all the commands synchronously.
    // glFinish();
}
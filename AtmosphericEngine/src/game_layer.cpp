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
#include "rmlui_manager.hpp"

GameLayer::GameLayer(Application* app) : Layer("GameLayer"), _app(app) {
}

void GameLayer::OnAttach() {
    // Initialize RmlUi
    auto windowSize = _app->GetWindow()->GetFramebufferSize();
    RmlUiManager::Get()->Initialize(windowSize.width, windowSize.height);

    // Load example HUD (optional - you can load this in your game's OnLoad instead)
    // auto hud = RmlUiManager::Get()->LoadDocument("assets/ui/hud.rml");
    // if (hud) hud->Show();
}

void GameLayer::OnDetach() {
    // Shutdown RmlUi
    RmlUiManager::Get()->Shutdown();
}

void GameLayer::OnUpdate(float dt) {
    for (auto& entity : _app->GetEntities()) {
        entity->Tick(dt);
    }

    // Update RmlUi
    RmlUiManager::Get()->Update(dt);
}

void GameLayer::OnRender(float dt) {
    // Note that draw calls are asynchronous, which means they return immediately.
    // So the drawing time can only be calculated along with the image presenting.
    _app->GetGraphicsServer()->Render(dt);
    // Nevertheless, glFinish() can force the GPU process all the commands synchronously.
    // glFinish();

    // Render RmlUi on top of the 3D scene
    RmlUiManager::Get()->Render();
}

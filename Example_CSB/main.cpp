#include "Atmospheric.hpp"

class CSBDemo : public Application {
    using Application::Application;

    CSBLoader* csbLoader = nullptr;
    CSBLoadResult loadedScene;

    void OnLoad() override {
        // Initialize CSB loader
        csbLoader = new CSBLoader(this);

        // Set up orthographic camera for 2D view
        mainCamera = graphics.GetMainCamera();
        if (mainCamera) {
            mainCamera->SetOrthographic(800.0f, 600.0f, -100.0f, 100.0f);
            mainCamera->gameObject->SetPosition(glm::vec3(400.0f, 300.0f, 0.0f));
            mainCamera->Yaw(-glm::half_pi<float>());
        }

        // Try to load a CSB file
        // For POC, we'll create some sprites programmatically if no CSB exists
        CSBLoadConfig config;
        config.basePath = "assets/";
        config.defaultLayer = CanvasLayer::LAYER_WORLD;

        loadedScene = csbLoader->Load("assets/test_scene.csb", config);

        if (loadedScene.success) {
            console.Info(fmt::format("CSB loaded successfully! {} nodes created", loadedScene.allNodes.size()));
        } else {
            console.Warn(fmt::format("CSB load failed: {}", loadedScene.error));
            console.Info("Creating demo sprites programmatically...");
            CreateDemoSprites();
        }

        console.Info("CSB Demo - Press R to reload, ESC to quit");
        console.Info(fmt::format("Supported node types: {}", fmt::join(CSBLoader::GetSupportedNodeTypes(), ", ")));
    }

    void CreateDemoSprites() {
        // Create some demo sprites to show the system works
        // This simulates what would be loaded from a CSB file

        // Background sprite
        auto bg = CreateGameObject(glm::vec2(400.0f, 300.0f));
        SpriteProps bgProps;
        bgProps.size = glm::vec2(600.0f, 400.0f);
        bgProps.pivot = glm::vec2(0.5f, 0.5f);
        bgProps.color = glm::vec4(0.2f, 0.3f, 0.4f, 1.0f);
        bgProps.layer = CanvasLayer::LAYER_BACKGROUND;
        bg->AddComponent<SpriteComponent>(bgProps);
        bg->SetName("Background");

        // Character sprite
        auto character = CreateGameObject(glm::vec2(400.0f, 300.0f));
        SpriteProps charProps;
        charProps.size = glm::vec2(64.0f, 64.0f);
        charProps.pivot = glm::vec2(0.5f, 0.5f);
        charProps.color = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f);
        charProps.layer = CanvasLayer::LAYER_WORLD;
        character->AddComponent<SpriteComponent>(charProps);
        character->SetName("Character");

        // UI element
        auto uiPanel = CreateGameObject(glm::vec2(100.0f, 550.0f));
        SpriteProps uiProps;
        uiProps.size = glm::vec2(150.0f, 40.0f);
        uiProps.pivot = glm::vec2(0.0f, 1.0f);// Top-left anchor
        uiProps.color = glm::vec4(0.1f, 0.1f, 0.1f, 0.8f);
        uiProps.layer = CanvasLayer::LAYER_UI;
        uiPanel->AddComponent<SpriteComponent>(uiProps);
        uiPanel->SetName("UIPanel");

        // Some decorative sprites
        for (int i = 0; i < 5; i++) {
            auto deco = CreateGameObject(glm::vec2(150.0f + i * 120.0f, 150.0f));
            SpriteProps decoProps;
            decoProps.size = glm::vec2(32.0f, 32.0f);
            decoProps.pivot = glm::vec2(0.5f, 0.5f);
            decoProps.color = glm::vec4(
              0.5f + i * 0.1f, 0.3f + i * 0.1f, 0.8f - i * 0.1f, 1.0f
            );
            decoProps.layer = CanvasLayer::LAYER_WORLD_BACK;
            deco->AddComponent<SpriteComponent>(decoProps);
            deco->SetName(fmt::format("Deco_{}", i));
        }

        console.Info("Created 8 demo sprites");
    }

    void OnUpdate(float dt, float time) override {
        // Animate the character if it exists
        if (loadedScene.success) {
            auto it = loadedScene.nodesByName.find("Character");
            if (it != loadedScene.nodesByName.end()) {
                auto* character = it->second;
                glm::vec3 pos = character->GetPosition();
                pos.x = 400.0f + std::sin(time * 2.0f) * 100.0f;
                pos.y = 300.0f + std::cos(time * 3.0f) * 50.0f;
                character->SetPosition(pos);
            }
        }

        // Reload scene
        if (input.IsKeyPressed(Key::R)) {
            ReloadScene();
        }

        if (input.IsKeyDown(Key::ESCAPE)) {
            Quit();
        }
    }
};

int main(int argc, char* argv[]) {
    CSBDemo game({
      .windowTitle = "CSB Loader Demo",
      .windowWidth = 800,
      .windowHeight = 600,
      .useDefaultTextures = true,
      .useDefaultShaders = true,
    });
    game.Run();
    return 0;
}

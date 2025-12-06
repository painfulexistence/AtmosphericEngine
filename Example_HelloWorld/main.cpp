#include "Atmospheric.hpp"

class HelloWorld : public Application {
    using Application::Application;

    GameObject* cube;

    // World space sprites
    std::vector<GameObject*> worldSprites;

    // Screen space sprites
    std::vector<GameObject*> screenSprites;

    void OnLoad() override {
        SceneDef scene = {
            .materials = { { .baseMap = 0,
                             .normalMap = 1,
                             .aoMap = 2,
                             .roughnessMap = 3,
                             .diffuse = { 1., 1., 1. },
                             .specular = { .296648, .296648, .296648 },
                             .ambient = { .25, .20725, .20725 },
                             .shininess = 0.088 } },
        };
        LoadScene(scene);

        mainCamera->gameObject->SetPosition(glm::vec3(-10.0, 5.0, 0.0));

        // Create rotating cube
        auto cubeMesh = AssetManager::Get().CreateCubeMesh("CubeMesh", 1.0f);
        cubeMesh->SetMaterial(AssetManager::Get().GetMaterials()[0]);

        cube = CreateGameObject();
        cube->AddComponent<MeshComponent>(cubeMesh);

        // === World Space Sprites ===
        // These follow the 2D camera and use world coordinates
        glm::vec4 worldColors[] = {
            {1.0f, 0.3f, 0.3f, 0.8f},  // Red
            {0.3f, 1.0f, 0.3f, 0.8f},  // Green
            {0.3f, 0.3f, 1.0f, 0.8f},  // Blue
            {1.0f, 1.0f, 0.3f, 0.8f},  // Yellow
        };

        for (int i = 0; i < 4; i++) {
            auto* spriteObj = CreateGameObject();
            spriteObj->SetPosition(glm::vec3(100.0f + i * 120.0f, 200.0f, 0.0f));

            auto* sprite = spriteObj->AddComponent<SpriteComponent>(SpriteProps{
                .size = glm::vec2(80.0f, 80.0f),
                .pivot = glm::vec2(0.5f, 0.5f),
                .color = worldColors[i],
                .textureID = -1,  // No texture, solid color
                .layer = CanvasLayer::LAYER_WORLD,
            });

            worldSprites.push_back(spriteObj);
        }

        // === Screen Space Sprites (UI) ===
        // These stay fixed on screen regardless of camera
        glm::vec4 uiColors[] = {
            {1.0f, 0.5f, 0.0f, 0.9f},  // Orange
            {0.5f, 0.0f, 1.0f, 0.9f},  // Purple
            {0.0f, 1.0f, 1.0f, 0.9f},  // Cyan
        };

        for (int i = 0; i < 3; i++) {
            auto* uiObj = CreateGameObject();
            // Position in screen coordinates (top-left origin)
            uiObj->SetPosition(glm::vec3(20.0f + i * 70.0f, 20.0f, 0.0f));

            auto* sprite = uiObj->AddComponent<SpriteComponent>(SpriteProps{
                .size = glm::vec2(50.0f, 50.0f),
                .pivot = glm::vec2(0.0f, 0.0f),  // Top-left pivot for UI
                .color = uiColors[i],
                .textureID = -1,
                .layer = CanvasLayer::LAYER_UI,
            });

            screenSprites.push_back(uiObj);
        }

        console.Info(fmt::format("Game fully loaded in {:.1f} seconds", GetWindowTime()));
        console.Info("Press 1-5 to switch level blockouts, R to reload shaders");
        console.Info("Added 4 world-space sprites and 3 screen-space UI sprites");
    }

    void OnUpdate(float dt, float time) override {
        cube->SetPosition(glm::vec3(0.0f, 5.0f, std::cos(time) * 2.0f));
        cube->SetRotation(glm::vec3(0.0, time * 0.5, time * 1.0));

        // Animate world sprites (bounce up and down)
        for (size_t i = 0; i < worldSprites.size(); i++) {
            float offset = std::sin(time * 2.0f + i * 0.5f) * 30.0f;
            worldSprites[i]->SetPosition(glm::vec3(
                100.0f + i * 120.0f,
                200.0f + offset,
                0.0f
            ));
        }

        // Animate UI sprites (pulse size effect via color alpha)
        for (size_t i = 0; i < screenSprites.size(); i++) {
            float pulse = 0.7f + 0.3f * std::sin(time * 3.0f + i * 1.0f);
            auto* sprite = screenSprites[i]->GetComponent<SpriteComponent>();
            glm::vec4 color = sprite->GetColor();
            color.a = pulse;
            sprite->SetColor(color);
        }

        if (input.IsKeyDown(Key::R)) {
            AssetManager::Get().ReloadShaders();
        }
        if (input.IsKeyDown(Key::ESCAPE)) {
            Quit();
        }
    }
};

int main(int argc, char* argv[]) {
    HelloWorld game({
      .useDefaultTextures = true,
      .useDefaultShaders = true,
    });
    game.Run();
    return 0;
}
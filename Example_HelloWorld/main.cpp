#include "Atmospheric.hpp"

class HelloWorld : public Application {
    using Application::Application;

    GameObject* cube;

    // World space sprites (rendered by WorldCanvasPass with depth testing)
    std::vector<GameObject*> worldSprites;

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
        // These are rendered by WorldCanvasPass with depth testing
        // They will be occluded by 3D geometry in front of them
        glm::vec4 worldColors[] = {
            {1.0f, 0.3f, 0.3f, 0.8f},  // Red
            {0.3f, 1.0f, 0.3f, 0.8f},  // Green
            {0.3f, 0.3f, 1.0f, 0.8f},  // Blue
            {1.0f, 1.0f, 0.3f, 0.8f},  // Yellow
        };

        for (int i = 0; i < 4; i++) {
            auto* spriteObj = CreateGameObject();
            // Position in 3D world space
            spriteObj->SetPosition(glm::vec3(i * 2.0f - 3.0f, 2.0f, 3.0f));

            spriteObj->AddComponent<SpriteComponent>(SpriteProps{
                .size = glm::vec2(1.0f, 1.0f),  // World units
                .pivot = glm::vec2(0.5f, 0.5f),
                .color = worldColors[i],
                .textureID = -1,  // No texture, solid color
                .layer = CanvasLayer::LAYER_WORLD,
            });

            worldSprites.push_back(spriteObj);
        }

        console.Info(fmt::format("Game fully loaded in {:.1f} seconds", GetWindowTime()));
        console.Info("Press R to reload shaders, ESC to quit");
        console.Info("Added 4 world-space sprites (depth tested)");
    }

    void OnUpdate(float dt, float time) override {
        cube->SetPosition(glm::vec3(0.0f, 5.0f, std::cos(time) * 2.0f));
        cube->SetRotation(glm::vec3(0.0, time * 0.5, time * 1.0));

        // Animate world sprites (float up and down)
        for (size_t i = 0; i < worldSprites.size(); i++) {
            float offset = std::sin(time * 2.0f + i * 0.5f) * 0.5f;
            worldSprites[i]->SetPosition(glm::vec3(
                i * 2.0f - 3.0f,
                2.0f + offset,
                3.0f
            ));
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
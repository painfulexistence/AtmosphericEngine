#include "Atmospheric.hpp"
#include "Atmospheric/level_blockout.hpp"

class HelloWorld : public Application {
    using Application::Application;

    GameObject* cube;
    GameObject* levelBlockout;
    int currentLevel = 0;
    static constexpr int NUM_LEVELS = 5;

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

        mainCamera->gameObject->SetPosition(glm::vec3(-15.0, 10.0, 15.0));

        // Create rotating cube
        auto cubeMesh = AssetManager::Get().CreateCubeMesh("CubeMesh", 1.0f);
        cubeMesh->SetMaterial(AssetManager::Get().GetMaterials()[0]);

        cube = CreateGameObject();
        cube->AddComponent<MeshComponent>(cubeMesh);

        // Create CSG level blockout
        levelBlockout = CreateGameObject();
        BuildLevel(currentLevel);

        script.Print(fmt::format("Game fully loaded in {:.1f} seconds", GetWindowTime()));
        script.Print("Press 1-5 to switch level blockouts, R to reload shaders");
    }

    void BuildLevel(int levelIndex) {
        CSG::NodePtr levelNode;
        const char* levelName;

        switch (levelIndex) {
            case 0:
                levelNode = LevelBlockout::ExampleDungeonRoom();
                levelName = "Dungeon Room";
                break;
            case 1:
                levelNode = LevelBlockout::ExampleLCorridor();
                levelName = "L-Corridor";
                break;
            case 2:
                levelNode = LevelBlockout::ExampleRoomWithWindow();
                levelName = "Room with Window";
                break;
            case 3:
                levelNode = LevelBlockout::ExampleArena();
                levelName = "Arena";
                break;
            case 4:
            default:
                levelNode = LevelBlockout::ExampleDungeonLayout();
                levelName = "Dungeon Layout";
                break;
        }

        auto levelMesh = LevelBlockout::BuildLevelMesh(levelNode);
        levelMesh->SetMaterial(AssetManager::Get().GetMaterials()[0]);

        // Update or add mesh component
        auto* meshComp = levelBlockout->GetComponent<MeshComponent>();
        if (meshComp) {
            meshComp->SetMesh(levelMesh.get());
        } else {
            levelBlockout->AddComponent<MeshComponent>(levelMesh.get());
        }

        script.Print(fmt::format("Loaded level: {}", levelName));
    }

    void OnUpdate(float dt, float time) override {
        cube->SetPosition(glm::vec3(0.0f, 5.0f, std::cos(time) * 2.0f));
        cube->SetRotation(glm::vec3(0.0, time * 0.5, time * 1.0));

        // Level switching with number keys
        if (input.IsKeyDown(Key::Num1)) { currentLevel = 0; BuildLevel(currentLevel); }
        if (input.IsKeyDown(Key::Num2)) { currentLevel = 1; BuildLevel(currentLevel); }
        if (input.IsKeyDown(Key::Num3)) { currentLevel = 2; BuildLevel(currentLevel); }
        if (input.IsKeyDown(Key::Num4)) { currentLevel = 3; BuildLevel(currentLevel); }
        if (input.IsKeyDown(Key::Num5)) { currentLevel = 4; BuildLevel(currentLevel); }

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
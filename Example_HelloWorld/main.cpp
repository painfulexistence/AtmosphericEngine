#include "Atmospheric.hpp"

class HelloWorld : public Application {
    using Application::Application;

    GameObject* cube;

    void OnLoad() override {
        SceneDef scene = {
            .materials = {
                {
                    .baseMap = 0,
                    .normalMap = 1,
                    .aoMap = 2,
                    .roughnessMap = 3,
                    .diffuse = {1., 1., 1.},
                    .specular = {.296648, .296648, .296648},
                    .ambient = {.25, .20725, .20725},
                    .shininess = 0.088
                }
            },
        };
        LoadScene(scene);

        mainCamera->gameObject->SetPosition(glm::vec3(-5.0, 0.0, 0.0));

        auto cubeMesh = graphics.CreateCubeMesh("CubeMesh", 1.0f);
        cubeMesh->SetMaterial(graphics.materials[0]);

        cube = CreateGameObject();
        cube->AddRenderable(cubeMesh);

        script.Print(fmt::format("Game fully loaded in {:.1f} seconds", GetWindowTime()));
    }

    void OnUpdate(float dt, float time) override {
        cube->SetPosition(glm::vec3(0.0f, 0.0f, std::cos(time) * 2.0f));
        cube->SetRotation(glm::vec3(0.0, time * 0.5, time * 1.0));

        if (input.IsKeyDown(Key::R)) {
            graphics.ReloadShaders();
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
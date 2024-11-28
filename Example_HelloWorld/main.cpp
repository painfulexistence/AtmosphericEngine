#include "Atmospheric.hpp"

class HelloWorld : public Application {
    GameObject* cube;

    void OnLoad() override {
        mainCamera->gameObject->SetPosition(glm::vec3(-5.0, 0.0, 0.0));

        auto cubeMesh = graphics.CreateCubeMesh("CubeMesh", 1.0f);
        cubeMesh->SetMaterial(graphics.materials[1]);

        cube = CreateGameObject();
        cube->AddRenderable("CubeMesh");

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
    HelloWorld game;
    game.Run();
    return 0;
}
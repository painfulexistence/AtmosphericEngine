#include "Atmospheric.hpp"

class VoxelWorldApp : public Application {
    using Application::Application;

    VoxelWorld _world;

    void OnLoad() override {
        mainCamera->gameObject->SetPosition(glm::vec3(200.0f, 80.0f, 200.0f));
        mainCamera->gameObject->SetRotation(glm::vec3(glm::radians(-20.0f), 0.0f, 0.0f));

        _world.Init(this, /*seed=*/1337);

        console.Info("VoxelWorld loaded. WASD to move, ESC to quit.");
    }

    void OnUpdate(float dt, float time) override {
        // Camera fly controls
        glm::vec3 pos = mainCamera->gameObject->GetPosition();
        glm::vec3 rot = mainCamera->gameObject->GetRotation();
        const float speed = 20.0f;

        glm::vec3 forward = glm::vec3(
            std::sin(rot.y) * std::cos(rot.x),
            -std::sin(rot.x),
            std::cos(rot.y) * std::cos(rot.x)
        );
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

        if (input.IsKeyDown(Key::W)) pos += forward * speed * dt;
        if (input.IsKeyDown(Key::S)) pos -= forward * speed * dt;
        if (input.IsKeyDown(Key::A)) pos -= right   * speed * dt;
        if (input.IsKeyDown(Key::D)) pos += right   * speed * dt;
        if (input.IsKeyDown(Key::Q)) pos.y -= speed * dt;
        if (input.IsKeyDown(Key::E)) pos.y += speed * dt;

        mainCamera->gameObject->SetPosition(pos);

        _world.Update(dt, pos);

        glm::mat4 viewProj = mainCamera->GetProjectionMatrix() * mainCamera->GetViewMatrix();
        _world.SubmitRenderCommands(GetGraphicsServer()->renderer, viewProj, pos);

        if (input.IsKeyPressed(Key::R)) {
            AssetManager::Get().ReloadShaders();
        }
        if (input.IsKeyPressed(Key::ESCAPE)) {
            Quit();
        }
    }
};

int main(int argc, char* argv[]) {
    VoxelWorldApp game({
        .useDefaultTextures = true,
        .useDefaultShaders  = true,
    });
    game.Run();
    return 0;
}

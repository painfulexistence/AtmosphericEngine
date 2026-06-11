#include "Atmospheric.hpp"

class VoxelWorldApp : public Application {
    using Application::Application;

    VoxelWorld _world;

    void OnInit() override {
        GoScene("main", [this]{ OnLoad(); });
    }

    void OnLoad() override {
        LoadScene({});  // initializes mainCamera / mainLight

        mainCamera->gameObject->SetPosition(glm::vec3(200.0f, 80.0f, 200.0f));
        mainCamera->gameObject->SetRotation(glm::vec3(glm::radians(-20.0f), 0.0f, 0.0f));

        _world.Init(this, /*seed=*/1337);

        console.Info("VoxelWorld loaded. WASD move, RF up/down, IJKL look, ESC quit.");
    }

    void OnUpdate(float dt, float /*time*/) override {
        glm::vec3 pos = mainCamera->gameObject->GetPosition();
        glm::vec3 rot = mainCamera->gameObject->GetRotation();

        const float moveSpeed = 20.0f;
        const float lookSpeed = 1.5f; // radians/sec

        // IJKL look
        if (input.IsKeyDown(Key::I)) rot.x -= lookSpeed * dt;
        if (input.IsKeyDown(Key::K)) rot.x += lookSpeed * dt;
        if (input.IsKeyDown(Key::J)) rot.y -= lookSpeed * dt;
        if (input.IsKeyDown(Key::L)) rot.y += lookSpeed * dt;
        rot.x = glm::clamp(rot.x, glm::radians(-89.0f), glm::radians(89.0f));
        mainCamera->gameObject->SetRotation(rot);

        // Forward vector from current rotation
        glm::vec3 forward = glm::vec3(
            std::sin(rot.y) * std::cos(rot.x),
            -std::sin(rot.x),
            std::cos(rot.y) * std::cos(rot.x)
        );
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

        // WASD move, RF up/down
        if (input.IsKeyDown(Key::W)) pos += forward * moveSpeed * dt;
        if (input.IsKeyDown(Key::S)) pos -= forward * moveSpeed * dt;
        if (input.IsKeyDown(Key::A)) pos -= right   * moveSpeed * dt;
        if (input.IsKeyDown(Key::D)) pos += right   * moveSpeed * dt;
        if (input.IsKeyDown(Key::R)) pos.y += moveSpeed * dt;
        if (input.IsKeyDown(Key::F)) pos.y -= moveSpeed * dt;

        mainCamera->gameObject->SetPosition(pos);

        _world.Update(dt, pos);

        glm::mat4 viewProj = mainCamera->GetProjectionMatrix() * mainCamera->GetViewMatrix();
        _world.SubmitRenderCommands(GetGraphicsServer()->renderer, viewProj, pos);

        if (input.IsKeyPressed(Key::ESCAPE)) {
            Quit();
        }
    }
};

#ifdef __EMSCRIPTEN__
static void StartGame();

int main(int argc, char* argv[]) {
    FileSystem::Get().Prefetch({}, StartGame);
    return 0;
}

static void StartGame() {
    static VoxelWorldApp game({
        .useDefaultTextures = true,
        .useDefaultShaders  = true,
    });
    game.Run();
}
#else
int main(int argc, char* argv[]) {
    VoxelWorldApp game({
        .useDefaultTextures = true,
        .useDefaultShaders  = true,
    });
    game.Run();
    return 0;
}
#endif

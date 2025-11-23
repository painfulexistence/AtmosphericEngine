#include "Atmospheric.hpp"

class TerrainDemo : public Application {
    using Application::Application;

    int windowWidth, windowHeight;
    std::vector<float> terrainData;
    GameObject* flyCamGO;
    CameraComponent* flyCam;
    float cameraSpeed = 5.0f;
    float slowCameraSpeed = 1.0f;
    bool isSlowCamera = false;
    bool isWireframeEnabled = false;

    void OnLoad() override {
        srand(time(NULL));

        LoadScene({ .textures = { "./assets/textures/heightmap.jpg", "./assets/textures/heightmap_debug.jpg" },
                    .materials = { { .heightMap = 5,
                                     .diffuse = { 1., 1., 1. },
                                     .specular = { .7, .6, .6 },
                                     .ambient = { .0, .0, .0 },
                                     .shininess = 0.25 } } });

        auto windowSize = Window::Get()->GetFramebufferSize();
        windowWidth = windowSize.width;
        windowHeight = windowSize.height;

        graphics.EnableWireframe(isWireframeEnabled);

        flyCam = mainCamera;
        flyCamGO = flyCam->gameObject;
        flyCamGO->SetPosition(glm::vec3(0, 64, 0));
        // TODO: add togglable physics
        // auto flyCamMesh = graphics.CreateMesh("flyCam");
        // flyCamMesh->AddCapsuleShape(0.5f, 3.0f);
        // flyCamGO->AddRigidbody({
        //     .mass = 0.01f,
        //     .useGravity = false,
        //     .shape = graphics.GetMesh("flyCam")->GetShape()
        // });

        const float worldSize = 1024.f;
        const int worldResolution = 128;
        auto terrain = CreateGameObject(glm::vec3(0.0f, -10.0f, 0.0f));
        terrain->AddComponent<TerrainComponent>(
          &graphics,
          &physics,
          TerrainProps{ .heightmapPath = "assets/textures/heightmap.jpg",
                        .worldSize = worldSize,
                        .resolution = worldResolution,
                        .heightScale = 32.0f,
                        .minHeight = -64.0f,
                        .maxHeight = 64.0f,
                        .material = graphics.materials[0] }
        );
        auto terrainMesh = terrain->GetComponent<TerrainComponent>()->GetMesh();
        terrain->AddComponent<RigidbodyComponent>(RigidbodyProps{
          .mass = 0.0f, .friction = 1.0f, .restitution = 0.0f, .shape = terrainMesh->GetShape(), .useGravity = false });
    }

    void OnUpdate(float dt, float time) override {
        glm::vec3 currPos = flyCamGO->GetPosition();
        glm::vec3 currForward = flyCam->GetEyeDirection();
        if (input.IsKeyDown(Key::Z)) {
            isSlowCamera = true;
        } else {
            isSlowCamera = false;
        }
        if (input.IsKeyDown(Key::UP)) {
            flyCam->Pitch(CAMERA_ANGULAR_OFFSET);
        }
        if (input.IsKeyDown(Key::DOWN)) {
            flyCam->Pitch(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.IsKeyDown(Key::RIGHT)) {
            flyCam->Yaw(CAMERA_ANGULAR_OFFSET);
        }
        if (input.IsKeyDown(Key::LEFT)) {
            flyCam->Yaw(-CAMERA_ANGULAR_OFFSET);
        }
        float camSpeed = isSlowCamera ? slowCameraSpeed : cameraSpeed;
        if (input.IsKeyDown(Key::W)) {
            flyCamGO->SetPosition(currPos + currForward * camSpeed);
        }
        if (input.IsKeyDown(Key::S)) {
            flyCamGO->SetPosition(currPos - currForward * camSpeed);
        }
        if (input.IsKeyPressed(Key::SPACE)) {
            isWireframeEnabled = !isWireframeEnabled;
            graphics.EnableWireframe(isWireframeEnabled);
        }
        if (input.IsKeyPressed(Key::ESCAPE)) {
            Quit();
        }
    }
};

int main(int argc, char* argv[]) {
    TerrainDemo game({ .windowFloating = true, .useDefaultTextures = true });
    game.Run();
    return 0;
}
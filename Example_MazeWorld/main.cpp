#include "application.hpp"
#include <vector>
#include "stb_image.h"
using namespace std;

static vector<vector<bool>> generateMazeData(int size, int shouldConsumed);

enum GameScreen {
    MENU,
    PLAY,
    WIN,
    LOSE,
};

class MazeGame : public Application {
    bool isPhysicsDebugUIEnabled = false;
    bool isPostProcessEnabled = false;
    bool isWireframeEnabled = false;

    const int MAZE_SIZE = 30;
    const bool MAZE_ROOFED = false;
    const float TILE_SIZE = 3.0;
    const int TILES_TO_REMOVE = 500;
    const float CHISM_CHANCE = 10.0;

    bool isLightFlashing = false;
    glm::vec3 winCoord = glm::vec3(0, 0, 0);
    GameObject* player = nullptr;
    std::vector<float> terrainData;

    void Load() override {
        // Load models
        auto characterModel = new Mesh();
        characterModel->AddCapsuleShape(0.5f, 3.0f);
        Mesh::MeshList.insert({"Character", characterModel});

        auto skyboxModel = Mesh::CreateCube(800.0f);
        skyboxModel->SetMaterial(graphics.materials[1]);
        skyboxModel->cullFaceEnabled = false;
        Mesh::MeshList.insert({"Skybox", skyboxModel});

        const float worldSize = 1024.f;
        const int worldResolution = 128;
        auto terrainModel = Mesh::CreateTerrain(worldSize, worldResolution);
        terrainModel->SetMaterial(graphics.materials[6]);

        int w, h, numChannels;
        unsigned char* data = stbi_load("assets/textures/heightmap_debug.jpg", &w, &h, &numChannels, 0);
        if (data != nullptr) {
            const int terrainDataSize = w * h;
            terrainData.resize(terrainDataSize);
            for (int i = 0; i < terrainDataSize; ++i) {
                terrainData[i] = (static_cast<float>(data[i]) / 255.0f) * 32.0f;
            }
            terrainModel->SetShape(new btHeightfieldTerrainShape(w, h, terrainData.data(), 1.0f, -64.0f, 64.0f, 1, PHY_FLOAT, true));
            terrainModel->GetShape()->setLocalScaling(btVector3(10.0f, 1.0f, 10.0f));

            stbi_image_free(data);
        } else {
            throw std::runtime_error("Could not load heightmap");
        }
        Mesh::MeshList.insert({"Terrain", terrainModel});

        auto cubeModel = Mesh::CreateCubeWithPhysics((float)TILE_SIZE);
        cubeModel->SetMaterial(graphics.materials[3]);
        Mesh::MeshList.insert({"Cube", cubeModel});

        auto sphereModel = Mesh::CreateSphereWithPhysics();
        sphereModel->SetMaterial(graphics.materials[5]);
        Mesh::MeshList.insert({"Sphere", sphereModel});

        script.Print("Models loaded.");

        // Create game objects in scene
        player = cameras.at(0);
        player->SetPosition(glm::vec3(0, 64, 0));
        Impostor* rb = ComponentFactory::CreateImpostor(player, &physics, "Character", 10.0f);
        rb->SetLinearFactor(glm::vec3(1, 1, 1));
        rb->SetAngularFactor(glm::vec3(0, 0, 0));

        auto skybox = CreateGameObject();
        skybox->AddMesh("Skybox");

        auto terrain = CreateGameObject();
        terrain->SetPosition(glm::vec3(0.0f, -10.0f, 0.0f));
        // terrain->AddMesh("Terrain");
        terrain->AddImpostor("Terrain");

        for (int i = 0; i < 50; ++i) {
            float diameter = (float)(rand() % 10 + 1);

            auto sphere = CreateGameObject(); //glm::scale(glm::mat4(1.0), glm::vec3(diameter))
            sphere->SetPosition(glm::vec3(rand() % 10 - 5, rand() % 100 + 20, rand() % 10 - 5));
            sphere->AddMesh("Sphere");
            sphere->AddImpostor("Sphere", 1.0f);
        }

        bool characterPlaced = false;
        vector<vector<bool>> maze = generateMazeData(MAZE_SIZE, TILES_TO_REMOVE);
        for (int x = 0; x < MAZE_SIZE; x++) {
            for (int z = 0; z < MAZE_SIZE; z++) {
                if (MAZE_ROOFED) {
                    auto cube = CreateGameObject();
                    cube->SetPosition(TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 3, z - MAZE_SIZE / 2.f));
                    cube->AddMesh("Cube");
                    cube->AddImpostor("Cube", 0.0f);
                }
                if (maze[x][z]) {
                    for (int h = 0; h < 3; h++) {
                        auto cube = CreateGameObject();
                        cube->SetPosition(TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, h, z - MAZE_SIZE / 2.f));
                        cube->AddMesh("Cube");
                        cube->AddImpostor("Cube", 0.0f);
                    }
                } else {
                    if (rand() % 100 < CHISM_CHANCE) {
                        continue; //Create chism
                    }
                    if (!characterPlaced) {
                        // FIXME: SetModelWorldTransform is not working properly
                        player->SetModelWorldTransform(glm::translate(glm::mat4(1.0f), TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f)));
                        characterPlaced = true;
                    }
                    winCoord.x = TILE_SIZE * (x - MAZE_SIZE / 2.f);
                    winCoord.y = 0.f;
                    winCoord.z = TILE_SIZE * (z - MAZE_SIZE / 2.f);
                }
                auto cube = CreateGameObject();
                cube->SetPosition(TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, -1, z - MAZE_SIZE / 2.f));
                cube->AddMesh("Cube");
                cube->AddImpostor("Cube", 0.0f);
            }
        }

        script.Print("Scene & world loaded.");
        script.Print(fmt::format("Game fully loaded in {:.1f} seconds", GetWindowTime()));
    }

    void Update(float dt, float time) override {
        // Update environment
        if (isLightFlashing) {
            glm::vec3 col = glm::vec3(1, 1, 1);
            mainLight->diffuse = abs(glm::cos(glm::radians(10.0f) * time)) * (float)(rand() % 2 / 2.0 + 0.5) * col;
        }

        // Input handling
        glm::vec3 currentVel = player->GetVelocity();
        if (input.GetKeyDown(KEY_W)) {
            glm::vec3 v = mainCamera->GetDirection(Axis::FRONT);
            player->SetVelocity(glm::vec3(v.x, currentVel.y, v.z));
        }
        if (input.GetKeyDown(KEY_S)) {
            glm::vec3 v = mainCamera->GetDirection(Axis::FRONT);
            player->SetVelocity(glm::vec3(-v.x, currentVel.y, -v.z));
        }
        if (input.GetKeyDown(KEY_D)) {
            mainCamera->yaw(0.3 * CAMERA_ANGULAR_OFFSET);
            glm::vec3 v = mainCamera->GetDirection(Axis::RIGHT);
            player->SetVelocity(glm::vec3(v.x, currentVel.y, v.z));
        }
        if (input.GetKeyDown(KEY_A)) {
            mainCamera->yaw(-0.3 * CAMERA_ANGULAR_OFFSET);
            glm::vec3 v = mainCamera->GetDirection(Axis::RIGHT);
            player->SetVelocity(glm::vec3(-v.x, currentVel.y, -v.z));
        }
        if (input.GetKeyDown(KEY_UP)) {
            mainCamera->pitch(CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_DOWN)) {
            mainCamera->pitch(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_RIGHT)) {
            mainCamera->yaw(CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_LEFT)) {
            mainCamera->yaw(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_SPACE)) {
            currentVel = player->GetVelocity(); // update velcoity to reflect current horizontal speed
            glm::vec3 v = mainCamera->GetDirection(Axis::UP);
            player->SetVelocity(glm::vec3(currentVel.x, v.y, currentVel.z));
        }
        if (input.GetKeyDown(KEY_X)) {
            isLightFlashing = !isLightFlashing;
        }
        if (input.GetKeyDown(KEY_Z)) {
            player->FreezePhyisics();
        }
        if (input.GetKeyDown(KEY_I)) {
            physics.EnableDebugUI(!isPhysicsDebugUIEnabled);
            isPhysicsDebugUIEnabled = !isPhysicsDebugUIEnabled;
        }
        if (input.GetKeyDown(KEY_O)) {
            graphics.EnableWireframe(!isWireframeEnabled);
            isWireframeEnabled = !isWireframeEnabled;
        }
        if (input.GetKeyDown(KEY_P)) {
            graphics.EnablePostProcess(!isPostProcessEnabled);
            isPostProcessEnabled = !isPostProcessEnabled;
        }
        if (input.GetKeyDown(KEY_R)) {
            graphics.ReloadShaders();
        }
        if (input.GetKeyDown(KEY_ESCAPE)) {
            Quit();
        }
    }
};

static vector<vector<bool>> generateMazeData(int size, int shouldConsumed) {
    int mazeX = 1;
    int mazeY = 1;

    vector<vector<bool>> data(size);
    for (int i = 0; i < data.size(); ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            data[i].push_back(true);
        }
    }

    int tilesConsumed = 0;
    while (tilesConsumed < shouldConsumed && tilesConsumed < (size - 2) * (size - 2)) {
        int xDir = 0;
        int yDir = 0;
        if (rand() % 2 < 0.5) {
            xDir = rand() % 2 < 0.5 ? 1 : -1;
        } else {
            yDir = rand() % 2 < 0.5 ? 1 : -1;
        }
        int moves = rand() % (size - 1) + 1;
        for (int i = 0; i < moves; i++)
        {
            mazeX = max(1, min(mazeX + xDir, size - 2));
            mazeY = max(1, min(mazeY + yDir, size - 2));
            if (data[mazeX][mazeY])
            {
                data[mazeX][mazeY] = false;
                tilesConsumed++;
            }
        }
    }

    return data;
}


int main(int argc, char* argv[]) {
    //setbuf(stdout, NULL); // Cancel output stream buffering so that output can be seen immediately
    srand(time(NULL));

    MazeGame game;
    game.Run();

    return 0;
}


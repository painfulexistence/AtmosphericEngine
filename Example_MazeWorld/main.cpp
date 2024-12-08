#include "Atmospheric.hpp"

enum GameScreen {
    MENU,
    PLAY,
    WIN,
    LOSE,
};

class Player : public Component {
public:
    float health = 100.0f;
    float speed = 5.0f;
    bool isAlive = true;
    bool isJumping = false;
};

class Enemy : public Component {
public:
    float health = 100.0f;
    float speed = 5.0f;
    bool isAlive = true;

    void TakeDamage(float damage) {
        this->health -= damage;
        if (this->health <= 0) {
            this->gameObject->SetPhysicsActivated(false);
        }
    }
};

class Maze {
public:
    Maze(int width, int height, float blockSize, int numBlocksToRemove, bool isRoofed, float chismChance) : _width(width), _height(height), _data(width * height, 1), _blockSize(blockSize), _numBlocksToRemove(numBlocksToRemove), _isRoofed(isRoofed), _chismChance(chismChance) {
        int currX = 1, currZ = 1, blocksRemoved = 0;
        while (blocksRemoved < _numBlocksToRemove && blocksRemoved < (_width - 2) * (_height - 2)) {
            int xDir = 0, zDir = 0, moves = 0;
            if (rand() % 2 < 0.5) {
                xDir = rand() % 2 < 0.5 ? 1 : -1;
                moves = rand() % (_width - 1) + 1;
            } else {
                zDir = rand() % 2 < 0.5 ? 1 : -1;
                moves = moves = rand() % (_height - 1) + 1;
            }
            for (int i = 0; i < moves; i++) {
                currX = std::max(1, std::min(currX + xDir, _width - 2));
                currZ = std::max(1, std::min(currZ + zDir, _height - 2));
                if (_data[currZ * _width + currX]) {
                    _data[currZ * _width + currX] = false;
                    blocksRemoved++;
                }
            }
        }
    };

    bool IsEmpty(int x, int z) {
        return _data[z * _width + x] == 0;
    }

    std::shared_ptr<Mesh> GenerateMesh() {
        MeshBuilder b;
        std::vector<bool> visited(_width * _height, false);
        for (int x = 0; x < _width; x++) {
            for (int z = 0; z < _height; z++) {
                if (_isRoofed) {

                }
                if (visited[z * _width + x] || IsEmpty(x, z)) {
                    continue;
                }

                if (IsEmpty(x, z)) {

                } else {

                }
                int startX = x, startZ = z, endX = x, endZ = z;

                while (endX + 1 < _width && IsEmpty(endX + 1, z) == IsEmpty(x, z) && !visited[z * _width + endX + 1]) {
                    ++endX;
                }

                bool expand = true;
                while (expand && endZ + 1 < _height) {
                    for (int xx = startX; xx <= endX; ++xx) {
                        if (IsEmpty(x, endZ + 1) != IsEmpty(x, z) || visited[(endZ + 1) * _width + xx]) {
                            expand = false;
                            break;
                        }
                    }
                    if (expand) {
                        ++endZ;
                    }
                }

                for (int zz = startZ; zz <= endZ; ++zz) {
                    for (int xx = startX; xx <= endX; ++xx) {
                        visited[zz * _height + xx] = true;
                    }
                }

                float x0 = startX * _blockSize;
                float y0 = startZ * _blockSize;
                float x1 = (endX + 1) * _blockSize;
                float y1 = (endZ + 1) * _blockSize;
                b.PushQuad(); //TODO:
            }
        }
        auto mesh = b.Build();
        return mesh;
    };

private:
    const int _width;
    const int _height;
    const int _blockSize;
    const bool _isRoofed;
    const int _numBlocksToRemove;
    const float _chismChance;

    std::vector<uint8_t> _data;
};

class MazeGame : public Application {
    bool isPhysicsDebugUIEnabled = false;
    bool isPostProcessEnabled = false;
    bool isWireframeEnabled = false;

    uint16_t seed = time(NULL);
    bool isLightFlashing = false;
    glm::vec3 winCoord = glm::vec3(0, 0, 0);
    GameObject* player = nullptr;
    std::vector<float> terrainData;
    bool isPlayerJumping = false;
    float playerSpeed = 10.0f;
    float playerJumpSpeed = 6.0f;
    std::vector<GameObject*> bullets;
    const int numMaxBullets = 200;
    int currentBulletIndex = 0;
    MusicID bgm = 0;
    SoundID sfxShoot = 0;

    void OnLoad() override {
        srand(seed);

        LoadScene(script.GetScenes()[0]);

        // Load sounds
        bgm = audio.LoadMusic("assets/sounds/Lost Highway.mp3");
        audio.SetMusicVolume(bgm, 2.0f);
        audio.PlayMusic(bgm);
        sfxShoot = audio.LoadSound("assets/sounds/fire.wav");

        // Load models
        auto characterMesh = graphics.CreateMesh("Character");
        characterMesh->AddCapsuleShape(0.5f, 3.0f);

        auto skyboxMesh = graphics.CreateCubeMesh("Skybox", 800.0f);
        skyboxMesh->SetMaterial(graphics.materials[1]);
        skyboxMesh->GetMaterial()->cullFaceEnabled = false;

        const float worldSize = 1024.f;
        const int worldResolution = 128;
        auto terrainMesh = graphics.CreateTerrainMesh("Terrain", worldSize, worldResolution);
        terrainMesh->SetMaterial(graphics.materials[6]);
        auto img = AssetManager::loadImage("assets/textures/heightmap_debug.jpg");
        if (img) {
            const int terrainDataSize = img->width * img->height;
            terrainData.resize(terrainDataSize);
            for (int i = 0; i < terrainDataSize; ++i) {
                terrainData[i] = (static_cast<float>(img->byteArray[i]) / 255.0f) * 32.0f;
            }
            terrainMesh->SetShape(new btHeightfieldTerrainShape(img->width, img->height, terrainData.data(), 1.0f, -64.0f, 64.0f, 1, PHY_FLOAT, true));
            // terrainMesh->SetShapeLocalScaling(glm::vec3(10.0f, 1.0f, 10.0f));
        } else {
            throw std::runtime_error("Could not load heightmap");
        }

        auto bulletMesh = graphics.CreateMesh("Sphere", MeshBuilder::CreateSphereWithPhysics(0.1f, 12));
        bulletMesh->SetMaterial(graphics.materials[5]);

        script.Print("Models loaded.");

        // Create game objects in scene
        player = graphics.GetMainCamera()->gameObject;
        player->AddImpostor("Character", 10.0f, glm::vec3(1, 1, 1), glm::vec3(0, 0, 0));
        player->SetPosition(glm::vec3(0, 64, 0));
        player->SetCollisionCallback([](GameObject* other) {
            fmt::print("Player collided with game object {}!\n", other->GetName());
        });

        auto skybox = CreateGameObject();
        skybox->AddRenderable("Skybox");

        auto terrain = CreateGameObject(glm::vec3(0.0f, -10.0f, 0.0f));
        // terrain->AddRenderable("Terrain");
        // terrain->AddImpostor("Terrain");

        for (int i = 0; i < numMaxBullets; ++i) {
            glm::vec3 pos = glm::vec3(rand() % 10 - 5, rand() % 100 + 20, rand() % 10 - 5);
            auto bullet = CreateGameObject(pos);
            bullet->AddRenderable("Sphere");
            bullet->AddImpostor("Sphere", 1.0f);
            bullet->SetActive(false);
            bullet->SetCollisionCallback([](GameObject* other) {
                other->SetActive(false);
            });

            bullets.push_back(bullet);
        }

        const int MAZE_SIZE = 30;
        const float MAZE_BLOCK_SIZE = 3.0;
        const int BLOCKS_TO_REMOVE = 500;
        const bool MAZE_ROOFED = false;
        const float CHISM_CHANCE = 10.0;

        auto mazeBlockMesh = graphics.CreateMesh("MazeBlock", MeshBuilder::CreateCubeWithPhysics((float)MAZE_BLOCK_SIZE));
        mazeBlockMesh->SetMaterial(graphics.materials[3]);

        bool characterPlaced = false;
        auto maze = Maze(MAZE_SIZE, MAZE_SIZE, MAZE_BLOCK_SIZE, BLOCKS_TO_REMOVE,MAZE_ROOFED, CHISM_CHANCE);
        for (int x = 0; x < MAZE_SIZE; x++) {
            for (int z = 0; z < MAZE_SIZE; z++) {
                if (MAZE_ROOFED) {
                    auto cube = CreateGameObject(MAZE_BLOCK_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 3, z - MAZE_SIZE / 2.f));
                    cube->AddRenderable("MazeBlock");
                    cube->AddImpostor("MazeBlock", 0.0f);
                }
                if (!maze.IsEmpty(x, z)) {
                    for (int h = 0; h < 3; h++) {
                        auto cube = CreateGameObject(MAZE_BLOCK_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, h, z - MAZE_SIZE / 2.f));
                        cube->AddRenderable("MazeBlock");
                        cube->AddImpostor("MazeBlock", 0.0f);
                    }
                } else {
                    if (rand() % 100 < CHISM_CHANCE) {
                        continue; //Create chism
                    }
                    if (!characterPlaced) {
                        player->SetPosition(MAZE_BLOCK_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f));
                        characterPlaced = true;
                    }
                    winCoord.x = MAZE_BLOCK_SIZE * (x - MAZE_SIZE / 2.f);
                    winCoord.y = 0.f;
                    winCoord.z = MAZE_BLOCK_SIZE * (z - MAZE_SIZE / 2.f);
                }
                auto cube = CreateGameObject(MAZE_BLOCK_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, -1, z - MAZE_SIZE / 2.f));
                cube->AddRenderable("MazeBlock");
                cube->AddImpostor("MazeBlock", 0.0f);
            }
        }

        script.Print("Scene & world loaded.");
        script.Print(fmt::format("Game fully loaded in {:.1f} seconds", GetWindowTime()));
    }

    void OnUpdate(float dt, float time) override {
        // Update environment
        if (isLightFlashing) {
            glm::vec3 col = glm::vec3(1, 1, 1);
            mainLight->diffuse = abs(glm::cos(glm::radians(10.0f) * time)) * (float)(rand() % 2 / 2.0 + 0.5) * col;
        }

        // Input handling
        glm::vec3 currVel = player->GetVelocity();
        if (input.GetKeyDown(Key::Q)) {
            glm::vec3 forward = mainCamera->GetEyeDirection();
            glm::vec3 pos = mainCamera->GetEyePosition() + forward * 0.5f;
            glm::vec3 vel = forward * 50.0f;
            // bullets[currentBulletIndex]->SetActive(false);
            currentBulletIndex = (currentBulletIndex + 1) % numMaxBullets;
            bullets[currentBulletIndex]->SetActive(true);
            bullets[currentBulletIndex]->SetPosition(pos);
            bullets[currentBulletIndex]->SetVelocity(vel);

            audio.PlaySound(sfxShoot);
        }
        if (input.GetKeyDown(Key::W)) {
            glm::vec3 v = mainCamera->GetMoveVector(Axis::FRONT) * (float)CAMERA_SPEED;
            player->SetVelocity(glm::vec3(v.x, currVel.y, v.z));
        }
        if (input.GetKeyDown(Key::S)) {
            glm::vec3 v = mainCamera->GetMoveVector(Axis::FRONT) * playerSpeed;
            player->SetVelocity(glm::vec3(-v.x, currVel.y, -v.z));
        }
        if (input.GetKeyDown(Key::D)) {
            mainCamera->Yaw(0.3 * CAMERA_ANGULAR_OFFSET);
            glm::vec3 v = mainCamera->GetMoveVector(Axis::RIGHT) * playerSpeed;
            player->SetVelocity(glm::vec3(v.x, currVel.y, v.z));
        }
        if (input.GetKeyDown(Key::A)) {
            mainCamera->Yaw(-0.3 * CAMERA_ANGULAR_OFFSET);
            glm::vec3 v = mainCamera->GetMoveVector(Axis::RIGHT) * playerSpeed;
            player->SetVelocity(glm::vec3(-v.x, currVel.y, -v.z));
        }
        if (input.GetKeyDown(Key::SPACE) && !isPlayerJumping) {
            currVel = player->GetVelocity(); // update velcoity to reflect current horizontal speed
            glm::vec3 v = mainCamera->GetMoveVector(Axis::UP) * playerJumpSpeed;
            player->SetVelocity(glm::vec3(currVel.x, v.y, currVel.z));
            // TODO: implement jumping and grounded state
            // isPlayerJumping = true;
        }
        if (input.GetKeyDown(Key::UP)) {
            mainCamera->Pitch(CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(Key::DOWN)) {
            mainCamera->Pitch(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(Key::RIGHT)) {
            mainCamera->Yaw(CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(Key::LEFT)) {
            mainCamera->Yaw(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(Key::X)) {
            isLightFlashing = !isLightFlashing;
        }
        if (input.GetKeyDown(Key::Z)) {
            player->SetPhysicsActivated(false);
        }
        if (input.IsKeyPressed(Key::I)) {
            physics.EnableDebugUI(!isPhysicsDebugUIEnabled);
            isPhysicsDebugUIEnabled = !isPhysicsDebugUIEnabled;
        }
        if (input.IsKeyPressed(Key::O)) {
            graphics.EnableWireframe(!isWireframeEnabled);
            isWireframeEnabled = !isWireframeEnabled;
        }
        if (input.IsKeyPressed(Key::P)) {
            graphics.EnablePostProcess(!isPostProcessEnabled);
            isPostProcessEnabled = !isPostProcessEnabled;
        }
        if (input.IsKeyPressed(Key::R)) {
            ReloadScene();
        }
        if (input.IsKeyPressed(Key::ESCAPE)) {
            Quit();
        }
    }
};


int main(int argc, char* argv[]) {
    MazeGame game;
    game.Run();

    return 0;
}


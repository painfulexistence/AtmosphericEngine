#include "application.hpp"
using namespace std;

static vector<vector<bool>> generateMazeData(int size, int shouldConsumed);

class MazeGame : public Application
{
    sol::table initState;
    bool isLightFlashing = false;
    glm::vec3 winCoord = glm::vec3(0, 0, 0);
    GameObject* player = nullptr;

    void Load()
    {
        // Read data from script
        script.GetData(std::string("init_game_state"), initState);
        const int MAZE_SIZE = (int)initState["maze_size"];
        const float TILE_SIZE = (float)initState["tile_size"];
        const int TILES_TO_REMOVE = (int)initState["tiles_to_remove"];
        float chismProb = (float)initState["chism_probability"];
        bool mazeRoofed = (bool)initState["maze_roofed"];
        isLightFlashing = (bool)initState["is_light_flashing"];

        player = cameras.at(0);

        // Load models
        auto characterModel = new Mesh();
        characterModel->collisionShape = new btCapsuleShape(0.5f, 3.0f);
        Mesh::MeshList.insert({"Character", characterModel});

        auto skyboxModel = Mesh::CreateCube(800.0f);
        skyboxModel->material = graphics.materials[1];
        skyboxModel->cullFaceEnabled = false;
        Mesh::MeshList.insert({"Skybox", skyboxModel});

        auto terrainModel = Mesh::CreateTerrain(MAZE_SIZE * TILE_SIZE, 10, std::vector<GLfloat>(100, 0.0f));
        terrainModel->material = graphics.materials[1];
        terrainModel->collisionShape = new btBoxShape(btVector3(MAZE_SIZE * TILE_SIZE, 0.2f, MAZE_SIZE * TILE_SIZE));
        Mesh::MeshList.insert({"Terrain", terrainModel});

        auto cubeModel = Mesh::CreateCube((float)TILE_SIZE);
        cubeModel->material = graphics.materials[3];
        cubeModel->collisionShape = new btBoxShape(0.5f * btVector3(TILE_SIZE, TILE_SIZE, TILE_SIZE));
        Mesh::MeshList.insert({"Cube", cubeModel});

        auto sphereModel = Mesh::CreateSphere();
        sphereModel->material = graphics.materials[0];
        sphereModel->collisionShape = new btSphereShape(0.5f);
        Mesh::MeshList.insert({"Sphere", sphereModel});

        script.Print("Models loaded.");

        // Create game objects in scene
        player->SetPosition(glm::vec3(0, 2, 0));
        Impostor* rb = ComponentFactory::CreateImpostor(player, &physics, "Character", 10.0f);
        rb->SetLinearFactor(glm::vec3(1, 1, 1));
        rb->SetAngularFactor(glm::vec3(0, 0, 0));

        auto skybox = new GameObject();
        ComponentFactory::CreateMesh(skybox, &graphics, "Skybox");
        gameObjects.push_back(skybox);

        auto terrain = new GameObject();
        terrain->SetPosition(glm::vec3(0.0f, -10.0f, 0.0f));
        ComponentFactory::CreateMesh(terrain, &graphics, "Terrain");
        ComponentFactory::CreateImpostor(terrain, &physics, "Terrain");
        gameObjects.push_back(terrain);

        for (int i = 0; i < 50; ++i)
        {
            float diameter = (float)(rand() % 10 + 1);

            auto sphere = new GameObject(); //glm::scale(glm::mat4(1.0), glm::vec3(diameter))
            sphere->SetPosition(glm::vec3(rand() % 10 - 5, rand() % 100 + 20, rand() % 10 - 5));
            ComponentFactory::CreateMesh(sphere, &graphics, "Sphere");
            ComponentFactory::CreateImpostor(sphere, &physics, "Sphere", 1.0f);
            gameObjects.push_back(sphere);
        }

        bool characterPlaced = false;
        vector<vector<bool>> maze = generateMazeData(MAZE_SIZE, TILES_TO_REMOVE);
        for (int x = 0; x < MAZE_SIZE; x++)
        {
            for (int z = 0; z < MAZE_SIZE; z++)
            {
                if (mazeRoofed)
                {
                    auto cube = new GameObject();
                    cube->SetPosition(TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 3, z - MAZE_SIZE / 2.f));
                    ComponentFactory::CreateMesh(cube, &graphics, "Cube");
                    ComponentFactory::CreateImpostor(cube, &physics, "Cube", 0.0f);
                    gameObjects.push_back(cube);
                }
                if (maze[x][z])
                {
                    for (int h = 0; h < 3; h++)
                    {
                        auto cube = new GameObject();
                        cube->SetPosition(TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, h, z - MAZE_SIZE / 2.f));
                        ComponentFactory::CreateMesh(cube, &graphics, "Cube");
                        ComponentFactory::CreateImpostor(cube, &physics, "Cube", 0.0f);
                        gameObjects.push_back(cube);
                    }
                }
                else
                {
                    if (rand() % 100 < chismProb)
                    {
                        continue; //Create chism
                    }
                    if (!characterPlaced)
                    {
                        player->SetModelWorldTransform(glm::translate(glm::mat4(1.0f), TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f)));
                        characterPlaced = true;
                    }
                    winCoord.x = TILE_SIZE * (x - MAZE_SIZE / 2.f);
                    winCoord.y = 0.f;
                    winCoord.z = TILE_SIZE * (z - MAZE_SIZE / 2.f);
                }
                auto cube = new GameObject();
                cube->SetPosition(TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, -1, z - MAZE_SIZE / 2.f));
                ComponentFactory::CreateMesh(cube, &graphics, "Cube");
                ComponentFactory::CreateImpostor(cube, &physics, "Cube", 0.0f);
                gameObjects.push_back(cube);
            }
        }

        script.Print("Scene & world loaded.");
        script.Print(fmt::format("Game fully loaded in {:.1f} seconds", GetWindowTime()));
    }

    void Update(float dt, float time)
    {
        // Update environment
        if (isLightFlashing)
        {
            glm::vec3 col = glm::vec3(1, 1, 1);
            mainLight->diffuse = abs(glm::cos(glm::radians(10.0f) * time)) * (float)(rand() % 2 / 2.0 + 0.5) * col;
        }

        // Input handling
        Impostor* rb = dynamic_cast<Impostor*>(player->GetComponent("Physics"));
        if (rb == nullptr)
            throw runtime_error("Impostor not found");

        glm::vec3 currentVel = rb->GetLinearVelocity();
        if (input.GetKeyDown(KEY_W))
        {
            glm::vec3 v = mainCamera->CreateLinearVelocity(Axis::FRONT);
            rb->SetLinearVelocity(glm::vec3(v.x, currentVel.y, v.z));
        }
        if (input.GetKeyDown(KEY_S))
        {
            glm::vec3 v = mainCamera->CreateLinearVelocity(Axis::FRONT);
            rb->SetLinearVelocity(glm::vec3(-v.x, currentVel.y, -v.z));
        }
        if (input.GetKeyDown(KEY_D))
        {
            mainCamera->yaw(0.3 * CAMERA_ANGULAR_OFFSET);
            glm::vec3 v = mainCamera->CreateLinearVelocity(Axis::RIGHT);
            rb->SetLinearVelocity(glm::vec3(v.x, currentVel.y, v.z));
        }
        if (input.GetKeyDown(KEY_A))
        {
            mainCamera->yaw(-0.3 * CAMERA_ANGULAR_OFFSET);
            glm::vec3 v = mainCamera->CreateLinearVelocity(Axis::RIGHT);
            rb->SetLinearVelocity(glm::vec3(-v.x, currentVel.y, -v.z));
        }
        if (input.GetKeyDown(KEY_UP))
        {
            mainCamera->pitch(CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_DOWN))
        {
            mainCamera->pitch(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_RIGHT))
        {
            mainCamera->yaw(CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_LEFT))
        {
            mainCamera->yaw(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_SPACE))
        {
            currentVel = rb->GetLinearVelocity(); // update velcoity to reflect current horizontal speed
            glm::vec3 v = mainCamera->CreateLinearVelocity(Axis::UP);
            rb->SetLinearVelocity(glm::vec3(currentVel.x, v.y, currentVel.z));
        }
        if (input.GetKeyDown(KEY_X))
        {
            isLightFlashing = !isLightFlashing;
        }
        if (input.GetKeyDown(KEY_ESCAPE))
        {
            input.ReceiveMessage(MessageType::ON_QUIT);
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


int main(int argc, char* argv[])
{
    //setbuf(stdout, NULL); // Cancel output stream buffering so that output can be seen immediately
    srand(time(NULL));

    MazeGame game;
    game.Run();

    return 0;
}


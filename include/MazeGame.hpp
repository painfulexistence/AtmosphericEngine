#include "Runtime.hpp"
#include "Scripting/Lua.hpp"
using namespace std;

static vector<vector<bool>> generateMazeData(int size, int shouldConsumed);
static glm::mat4 ConvertPhysicalMatrix(const btTransform& trans);

class MazeGame : public Runtime
{    
public:
    sol::table maze, cameraTable, lightTable, textureTable, materialTable, shaderTable, gameState;

    void Load()
    {
        // Read data from script
        script.GetData(std::string("maze"), maze);
        script.GetData(std::string("cameras"), cameraTable);
        script.GetData(std::string("lights"), lightTable);
        script.GetData(std::string("textures"), textureTable);
        script.GetData(std::string("materials"), materialTable);
        script.GetData(std::string("shaders"), shaderTable);
        script.GetData(std::string("game_state"), gameState);

        // Load textures
        std::vector<std::string> paths;
        for (const auto& kv : textureTable)
        {
            sol::table tex = (sol::table)kv.second;
            paths.push_back((std::string)tex["path"]);
        }
        graphics.LoadTextures(paths);
        script.Print("Textures loaded.");

        // Create cameras
        for (const auto& kv : cameraTable)
        {
            Camera cam = Camera(CameraProps((sol::table)kv.second));
            auto capsuleShape = new btCapsuleShape(0.5f, 3.0f);
            cam.SetGraphicsId(scene.CreateGhostGeometry());
            cam.SetPhysicsId(physics.World()->CreateImpostor(capsuleShape, btVector3(0.f, 2.f, 0.f), 10.0f));
            cameras.push_back(cam);

            physics.World()->SetImpostorLinearFactor(cam.GetPhysicsId(), btVector3(1, 1, 1));
            physics.World()->SetImpostorAngularFactor(cam.GetPhysicsId(), btVector3(0, 0, 0));
            entities.push_back(cam);
        }
        script.Print("Cameras initialized.");

        // Create lights
        for (const auto& kv : lightTable)
        {
            Light light = Light(LightProps((sol::table)kv.second));
            lights.push_back(light); 
        }
        script.Print("Lights initialized.");

        // Create material
        for (const auto& kv : materialTable)
        {
            Material mat = Material((sol::table)kv.second);
            materials.push_back(mat);
        }
        script.Print("Materials initialized.");

        // Create shader programs
        colorProgram = ShaderProgram(shaderTable["color"]["vert"], shaderTable["color"]["frag"]);
        depthTextureProgram = ShaderProgram(shaderTable["depth"]["vert"], shaderTable["depth"]["frag"]);
        depthCubemapProgram = ShaderProgram(shaderTable["depth_cubemap"]["vert"], shaderTable["depth_cubemap"]["frag"]);
        hdrProgram = ShaderProgram(shaderTable["hdr"]["vert"], shaderTable["hdr"]["frag"]);
        script.Print("Shaders initialized.");

        graphics.BindSceneVAO();
        // Create meshes in scene
        {
            auto mesh = make_shared<Mesh>();
            mesh->AsCube(800.0f);
            mesh->material = materials[1];
            mesh->cullFaceEnabled = false;
            Scene::MeshTable.insert({"Skybox", mesh});
            
            auto ent = Entity();
            ent.SetGraphicsId(scene.CreateMeshGeometry("Skybox"));
            entities.push_back(ent);
        }
        {
            auto mesh = make_shared<Mesh>();
            mesh->AsSphere();
            mesh->material = materials[0];
            Scene::MeshTable.insert({"Sphere", mesh});

            for (int i = 0; i < 50; ++i)
            {
                float diameter = (float)(rand() % 10 + 1);
                auto sphereShape = new btSphereShape(btScalar(diameter));
                shapeCollection.push_back(sphereShape);

                auto ent = Entity();
                ent.SetGraphicsId(scene.CreateMeshGeometry("Sphere", glm::scale(glm::mat4(1.0), glm::vec3(diameter))));
                ent.SetPhysicsId(physics.World()->CreateImpostor(sphereShape, btVector3(rand() % 10 - 5, rand() % 100 + 20, rand() % 10 - 5), 1.0f));
                entities.push_back(ent);
            }
        }
        CreateMaze();
        script.Print("Scene & world loaded.");
        script.Print(fmt::format("Game fully loaded in {:.1f} seconds", Time()));
    }

    void Update(float dt, float time)
    {
        // Update environment
        if ((bool)gameState["is_light_flashing"])
        {
            glm::vec3 col = glm::vec3(gameState["light_color"][1], gameState["light_color"][2], gameState["light_color"][3]);
            lights[0].diffuse = abs(glm::cos(glm::radians(10.0f) * time)) * (float)(rand() % 2 / 2.0 + 0.5) * col;
        }

        // Input handling
        const uint64_t& impostor = cameras[0].GetPhysicsId();

        btVector3 currentVel;    
        if (!physics.World()->GetImpostorLinearVelocity(impostor, currentVel)) 
            throw runtime_error("Impostor not found");

        if (input.GetKeyDown(KEY_W))
        {
            glm::vec3 v = cameras[0].CreateLinearVelocity(Axis::FRONT);
            physics.World()->SetImpostorLinearVelocity(impostor, btVector3(v.x, currentVel.y(), v.z));
        }
        if (input.GetKeyDown(KEY_S))
        {
            glm::vec3 v = cameras[0].CreateLinearVelocity(Axis::FRONT);
            physics.World()->SetImpostorLinearVelocity(impostor, btVector3(-v.x, currentVel.y(), -v.z));
        }
        if (input.GetKeyDown(KEY_D))
        {
            cameras[0].yaw(0.3 * CAMERA_ANGULAR_OFFSET);
            glm::vec3 v = cameras[0].CreateLinearVelocity(Axis::RIGHT);
            physics.World()->SetImpostorLinearVelocity(impostor, btVector3(v.x, currentVel.y(), v.z));
        }
        if (input.GetKeyDown(KEY_A))
        {
            cameras[0].yaw(-0.3 * CAMERA_ANGULAR_OFFSET);
            glm::vec3 v = cameras[0].CreateLinearVelocity(Axis::RIGHT);
            physics.World()->SetImpostorLinearVelocity(impostor, btVector3(-v.x, currentVel.y(), -v.z));
        }
        if (input.GetKeyDown(KEY_UP))
        {
            cameras[0].pitch(CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_DOWN))
        {
            cameras[0].pitch(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_RIGHT))
        {
            cameras[0].yaw(CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_LEFT))
        {
            cameras[0].yaw(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_SPACE)) 
        {
            physics.World()->GetImpostorLinearVelocity(impostor, currentVel); // update velcoity to reflect current horizontal speed
            glm::vec3 v = cameras[0].CreateLinearVelocity(Axis::UP);
            physics.World()->SetImpostorLinearVelocity(impostor, btVector3(currentVel.x(), v.y, currentVel.z()));
        }
        if (input.GetKeyDown(KEY_X))
        {
            gameState["is_light_flashing"] = !(bool)gameState["is_light_flashing"];
        }
        if (input.GetKeyDown(KEY_ESCAPE))
        {
            input.ReceiveMessage(MessageType::ON_QUIT);
        }
    }

private:
    void CreateMaze()
    {
        const int MAZE_SIZE = maze["size"];
        const float TILE_SIZE = maze["tile_size"];
        const bool MAZE_ROOFED = maze["roofed"];
        const int TILES_TO_REMOVE = maze["tiles_to_remove"];
        const float CHISM_PROBABILITY = maze["chism_probability"];
        const auto& WIN_COORD = maze["win_coord"];

        {
            auto mesh = make_shared<Mesh>();
            mesh->AsCube((float)TILE_SIZE);
            mesh->material = materials[2];
            Scene::MeshTable.insert({"Cube", mesh});
            auto blockShape = new btBoxShape(0.5f * btVector3(TILE_SIZE, TILE_SIZE, TILE_SIZE));
            shapeCollection.push_back(blockShape);

            bool characterPlaced = false;
            vector<vector<bool>> maze = generateMazeData(MAZE_SIZE, TILES_TO_REMOVE);
            for (int x = 0; x < MAZE_SIZE; x++) {
                for (int z = 0; z < MAZE_SIZE; z++) {
                    if (MAZE_ROOFED) 
                    {
                        auto ent = Entity();
                        ent.SetGraphicsId(scene.CreateMeshGeometry("Cube"));
                        ent.SetPhysicsId(physics.World()->CreateImpostor(blockShape, TILE_SIZE * btVector3(x - MAZE_SIZE / 2.f, 3, z - MAZE_SIZE / 2.f), 0.0f));
                        entities.push_back(ent);
                    }
                    if (maze[x][z]) 
                    {
                        for (int h = 0; h < 3; h++)
                        {
                            auto ent = Entity();
                            ent.SetGraphicsId(scene.CreateMeshGeometry("Cube"));
                            ent.SetPhysicsId(physics.World()->CreateImpostor(blockShape, TILE_SIZE * btVector3(x - MAZE_SIZE / 2.f, h, z - MAZE_SIZE / 2.f)));
                            entities.push_back(ent);
                        }
                    } 
                    else 
                    {
                        if (rand() % 100 < CHISM_PROBABILITY) 
                        { 
                            continue; //Create chism
                        }
                        if (!characterPlaced)
                        {
                            scene.SetGeometryModelWorldTransform(cameras[0].GetGraphicsId(), glm::translate(glm::mat4(1.0f), TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f)));
                            characterPlaced = true;
                        }
                        WIN_COORD[1] = TILE_SIZE * (x - MAZE_SIZE / 2.f);
                        WIN_COORD[2] = 0.f;
                        WIN_COORD[3] = TILE_SIZE * (z - MAZE_SIZE / 2.f);
                    }
                    auto ent = Entity();
                    ent.SetGraphicsId(scene.CreateMeshGeometry("Cube"));
                    ent.SetPhysicsId(physics.World()->CreateImpostor(blockShape, TILE_SIZE * btVector3(x - MAZE_SIZE / 2.f, -1, z - MAZE_SIZE / 2.f)));
                    entities.push_back(ent);
                }
            }
        }
        {
            float size = MAZE_SIZE * TILE_SIZE;

            auto mesh = make_shared<Mesh>();
            mesh->AsTerrain(size, 10, std::vector<GLfloat>(100, 0.0f));
            mesh->material = materials[1];
            Scene::MeshTable.insert({"Terrain", mesh});
            auto terrainShape = new btBoxShape(btVector3(size, 0.2f, size));
            shapeCollection.push_back(terrainShape);

            auto ent = Entity();
            ent.SetGraphicsId(scene.CreateMeshGeometry("Terrain"));
            ent.SetPhysicsId(physics.World()->CreateImpostor(terrainShape, btVector3(0.0f, -10.0f, 0.0f)));
            entities.push_back(ent);
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
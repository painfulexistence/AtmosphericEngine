#include "Runtime.hpp"
using namespace std;

static vector<vector<bool>> generateMazeData(int size, int shouldConsumed);
static glm::mat4 ConvertPhysicalMatrix(const btTransform& trans);

class MazeGame : public Runtime
{
private:
    void CreateMaze()
    {
        script.LuaEnv();
        const auto& t = script.LuaEnv()["maze"];
        const int MAZE_SIZE = t["size"];
        const float TILE_SIZE = t["tile_size"];
        const bool MAZE_ROOFED = t["roofed"];
        const int TILES_TO_REMOVE = t["tiles_to_remove"];
        const float CHISM_PROBABILITY = t["chism_probability"];
        const auto& WIN_COORD = t["win_coord"];

        {
            auto mesh = make_shared<Mesh>();
            mesh->AsCube((float)TILE_SIZE);
            mesh->material = _materials[2];
            Scene::MeshTable.insert({"Cube", mesh});

            bool characterPlaced = false;
            vector<vector<bool>> maze = generateMazeData(MAZE_SIZE, TILES_TO_REMOVE);
            for (int x = 0; x < MAZE_SIZE; x++) {
                for (int z = 0; z < MAZE_SIZE; z++) {
                    if (MAZE_ROOFED) 
                    {
                        auto ent = Entity();
                        ent.SetGraphicsId(scene.CreateMeshGeometry("Cube"));
                        ent.SetPhysicsId(world.CreateBoxImpostor(TILE_SIZE * btVector3(x - MAZE_SIZE / 2.f, 3, z - MAZE_SIZE / 2.f), btVector3(TILE_SIZE, TILE_SIZE, TILE_SIZE)));
                        entities.push_back(ent);
                    }
                    if (maze[x][z]) 
                    {
                        for (int h = 0; h < 3; h++)
                        {
                            auto ent = Entity();
                            ent.SetGraphicsId(scene.CreateMeshGeometry("Cube"));
                            ent.SetPhysicsId(world.CreateBoxImpostor(TILE_SIZE * btVector3(x - MAZE_SIZE / 2.f, h, z - MAZE_SIZE / 2.f), btVector3(TILE_SIZE, TILE_SIZE, TILE_SIZE)));
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
                            scene.SetGeometryModelWorldTransform(_cameras[0].GetGraphicsId(), glm::translate(glm::mat4(1.0f), TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f)));
                            characterPlaced = true;
                        }
                        WIN_COORD[1] = TILE_SIZE * (x - MAZE_SIZE / 2.f);
                        WIN_COORD[2] = 0.f;
                        WIN_COORD[3] = TILE_SIZE * (z - MAZE_SIZE / 2.f);
                    }
                    auto ent = Entity();
                    ent.SetGraphicsId(scene.CreateMeshGeometry("Cube"));
                    ent.SetPhysicsId(world.CreateBoxImpostor(TILE_SIZE * btVector3(x - MAZE_SIZE / 2.f, -1, z - MAZE_SIZE / 2.f), btVector3(TILE_SIZE, TILE_SIZE, TILE_SIZE)));
                    entities.push_back(ent);
                }
            }
        }
        {
            float size = MAZE_SIZE * TILE_SIZE;

            auto mesh = make_shared<Mesh>();
            mesh->AsTerrain(size, 10, std::vector<GLfloat>(100, 0.0f));
            mesh->material = _materials[1];
            Scene::MeshTable.insert({"Terrain", mesh});

            auto ent = Entity();
            ent.SetGraphicsId(scene.CreateMeshGeometry("Terrain"));
            ent.SetPhysicsId(world.CreateBoxImpostor(btVector3(0.0f, -10.0f, 0.0f), btVector3(size, 0.2f, size)));
            entities.push_back(ent);
        }
    }
    
public:
    void Load()
    {
        // Create cameras
        sol::table cameras = script.LuaEnv()["cameras"].force();
        for (const auto& kv : cameras)
        {
            Camera cam = Camera((sol::table)kv.second);
            cam.SetGraphicsId(scene.CreateGhostGeometry());
            cam.SetPhysicsId(world.CreateCapsuleImpostor(btVector3(0.f, 2.f, 0.f), 1.0f, 4.0f, 10.0f));
            _cameras.push_back(cam);

            world.SetImpostorLinearFactor(cam.GetPhysicsId(), btVector3(1, 1, 1));
            world.SetImpostorAngularFactor(cam.GetPhysicsId(), btVector3(0, 0, 0));
            entities.push_back(cam);
        }
        script.Print("Cameras initialized.");

        // Create lights
        sol::table lights = script.LuaEnv()["lights"].force();
        for (const auto& kv : lights)
        {
            Light light = Light((sol::table)kv.second);
            _lights.push_back(light); 
        }
        script.Print("Lights initialized.");

        //Create textures
        sol::table textures = script.LuaEnv()["textures"].force();
        for (const auto& kv : textures)
        {
            Texture texture = Texture((sol::table)kv.second);
            renderer.CreateTexture(texture.path);
        }
        script.Print("Textures initialized.");

        // Create material
        sol::table materials = script.LuaEnv()["materials"].force();
        for (const auto& kv : materials)
        {
            Material mat = Material((sol::table)kv.second);
            _materials.push_back(mat);
        }
        script.Print("Materials initialized.");

        // Create shader programs
        sol::table shaders = script.LuaEnv()["shaders"].force();
        colorProgram = ShaderProgram(shaders["color"]["vert"], shaders["color"]["frag"]);
        depthTextureProgram = ShaderProgram(shaders["depth"]["vert"], shaders["depth"]["frag"]);
        depthCubemapProgram = ShaderProgram(shaders["depth_cubemap"]["vert"], shaders["depth_cubemap"]["frag"]);
        hdrProgram = ShaderProgram(shaders["hdr"]["vert"], shaders["hdr"]["frag"]);
        script.Print("Shader programs initialized.");

        renderer.BindSceneVAO();
        // Create meshes in scene
        {
            auto mesh = make_shared<Mesh>();
            mesh->AsCube(800.0f);
            mesh->material = _materials[1];
            mesh->cullFaceEnabled = false;
            Scene::MeshTable.insert({"Skybox", mesh});
            
            auto ent = Entity();
            ent.SetGraphicsId(scene.CreateMeshGeometry("Skybox"));
            entities.push_back(ent);
        }
        {
            auto mesh = make_shared<Mesh>();
            mesh->AsSphere();
            mesh->material = _materials[0];
            Scene::MeshTable.insert({"Sphere", mesh});

            for (int i = 0; i < 50; ++i)
            {
                float diameter = (float)(rand() % 10 + 1);

                auto ent = Entity();
                ent.SetGraphicsId(scene.CreateMeshGeometry("Sphere", glm::scale(glm::mat4(1.0), glm::vec3(diameter))));
                ent.SetPhysicsId(world.CreateSphereImpostor(btVector3(rand() % 10 - 5, rand() % 100 + 20, rand() % 10 - 5), diameter, 1.0f));
                entities.push_back(ent);
            }
        }
        CreateMaze();
        script.Print("Scene & world initialized.");
    }

    void Update(float dt, float time)
    {
        // Update environment
        if ((bool)script.LuaEnv()["game_state"]["is_light_flashing"])
        {
            glm::vec3 col = glm::vec3(script.LuaEnv()["game_state"]["light_color"][1], script.LuaEnv()["game_state"]["light_color"][2], script.LuaEnv()["game_state"]["light_color"][3]);
            _lights[0].diffuse = abs(glm::cos(glm::radians(10.0f) * time)) * (float)(rand() % 2 / 2.0 + 0.5) * col;
        }

        // Input handling
        const std::uint64_t& impostor = _cameras[0].GetPhysicsId();

        btVector3 currentVel;    
        if (!world.GetImpostorLinearVelocity(impostor, currentVel)) 
            throw runtime_error("Impostor not found");

        if (input.GetKeyDown(KEY_W))
        {
            glm::vec3 v = _cameras[0].CreateLinearVelocity(Axis::FRONT);
            world.SetImpostorLinearVelocity(impostor, btVector3(v.x, currentVel.y(), v.z));
        }
        if (input.GetKeyDown(KEY_S))
        {
            glm::vec3 v = _cameras[0].CreateLinearVelocity(Axis::FRONT);
            world.SetImpostorLinearVelocity(impostor, btVector3(-v.x, currentVel.y(), -v.z));
        }
        if (input.GetKeyDown(KEY_D))
        {
            _cameras[0].yaw(0.3 * CAMERA_ANGULAR_OFFSET);
            glm::vec3 v = _cameras[0].CreateLinearVelocity(Axis::RIGHT);
            world.SetImpostorLinearVelocity(impostor, btVector3(v.x, currentVel.y(), v.z));
        }
        if (input.GetKeyDown(KEY_A))
        {
            _cameras[0].yaw(-0.3 * CAMERA_ANGULAR_OFFSET);
            glm::vec3 v = _cameras[0].CreateLinearVelocity(Axis::RIGHT);
            world.SetImpostorLinearVelocity(impostor, btVector3(-v.x, currentVel.y(), -v.z));
        }
        if (input.GetKeyDown(KEY_UP))
        {
            _cameras[0].pitch(CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_DOWN))
        {
            _cameras[0].pitch(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_RIGHT))
        {
            _cameras[0].yaw(CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_LEFT))
        {
            _cameras[0].yaw(-CAMERA_ANGULAR_OFFSET);
        }
        if (input.GetKeyDown(KEY_SPACE)) 
        {
            world.GetImpostorLinearVelocity(impostor, currentVel); // update velcoity to reflect current horizontal speed
            glm::vec3 v = _cameras[0].CreateLinearVelocity(Axis::UP);
            world.SetImpostorLinearVelocity(impostor, btVector3(currentVel.x(), v.y, currentVel.z()));
        }
        if (input.GetKeyDown(KEY_Z)) 
        {
            glm::vec2 pos = input.GetMouseUV();
            glm::vec3 diff = glm::vec3(1.0, pos.x, pos.y);

            _lights[0].diffuse = diff;
        }
        if (input.GetKeyDown(KEY_X))
        {
            script.LuaEnv()["game_state"]["is_light_flashing"] = !(bool)script.LuaEnv()["game_state"]["is_light_flashing"];
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
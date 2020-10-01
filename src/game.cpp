#include "game.hpp"
#define MAZE_SIZE 50
#define TILES_TO_REMOVE 1000
#define MAZE_ROOFED false
#define TILE_SIZE 3

bool** generateMazeData(int size) {
    int mazeX = 1;
    int mazeY = 1;

    bool** data = new bool*[size];
    for (int i = 0; i < size; i++) {
        data[i] = new bool[size];
        for (int j = 0; j < size; j++) {
            data[i][j] = true;
        }
    }

    int tilesConsumed = 0;
    while (tilesConsumed < TILES_TO_REMOVE && tilesConsumed < (size - 2) * (size - 2)) {
        int xDir = 0;
        int yDir = 0;
        if (rand() % 2 < 0.5) {
            xDir = rand() % 2 < 0.5 ? 1 : -1;
        } else {
            yDir = rand() % 2 < 0.5 ? 1 : -1;
        }
        
        int moves = rand() % (size - 1) + 1;
        for (int i = 0; i < moves; i++) {
            mazeX = std::max(1, std::min(mazeX + xDir, size - 2));
            mazeY = std::max(1, std::min(mazeY + yDir, size - 2));
            if (data[mazeX][mazeY]) {
                data[mazeX][mazeY] = false;
                tilesConsumed++;
            }
        }
    }
    return data;
}

glm::vec3 createMaze(bool** maze, std::list<Mesh*>& mazeBlocks, Camera*& camera, Light*& light, btDiscreteDynamicsWorld* world, Shader* shader) {
    
    mazeBlocks.clear();
    bool characterPlaced = false;
    glm::vec3 charPos = glm::vec3(0, 0, 0);
    glm::vec3 winPos = glm::vec3(0, 0, 0);
    Cube* tile;

    for (int x = 0; x < MAZE_SIZE; x++) {
        for (int z = 0; z < MAZE_SIZE; z++) {
            if (MAZE_ROOFED) {
                tile = new Cube(TILE_SIZE, shader);
                mazeBlocks.push_back(tile);
                tile->AddRigidBody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 3, z - MAZE_SIZE / 2.f), world, 0.f);
            }
            if (maze[x][z]) {
                tile = new Cube(TILE_SIZE, shader);
                mazeBlocks.push_back(tile);
                tile->AddRigidBody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 0, z - MAZE_SIZE / 2.f), world, 0.f);
                
                tile = new Cube(TILE_SIZE, shader);
                mazeBlocks.push_back(tile);
                tile->AddRigidBody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f), world, 0.f);

                tile = new Cube(TILE_SIZE, shader);
                mazeBlocks.push_back(tile);
                tile->AddRigidBody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 2, z - MAZE_SIZE / 2.f), world, 0.f);
            } else {
                if (rand() % 100 < 20) {
                    //Create chism
                    continue;
                }
                if (!characterPlaced) {
                    charPos = (float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f);
                    characterPlaced = true;
                }
                winPos = (float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 0, z - MAZE_SIZE / 2.f);
            }                
            tile = new Cube(TILE_SIZE, shader);
            mazeBlocks.push_back(tile);
            tile->AddRigidBody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, -1, z - MAZE_SIZE / 2.f), world, 0.f);
        }
    }
    camera->setPosition(charPos);
    light->setPosition(winPos);

    return winPos;
}

glm::vec3 restart(std::list<Mesh*>& mazeBlocks, Camera*& camera, Light*& light, btDiscreteDynamicsWorld* world, Shader* shader) {
    camera = new Camera(glm::vec3(0, 2, 0), 0.f, 0.f, glm::radians(45.0f), SCREEN_W / SCREEN_H, world);
    return createMaze(generateMazeData(MAZE_SIZE), mazeBlocks, camera, light, world, shader);
}

Game::Game(GLFWwindow* window, Shader* shader, btDiscreteDynamicsWorld* world)
{
    _window = window;
    _shader = shader;
    _world = world;
}

void Game::Init()
{
    camera = new Camera(glm::vec3(0, 2, 0), 0.f, 0.f, glm::radians(45.0f), SCREEN_W / SCREEN_H, _world);
    light = new Light(glm::vec3(20, 100, 20), lightColor);    
    skybox = new Cube(800, _shader);
    terrain = new Terrain(256, 10, new float[100]{0}, _shader);
    winPosition = createMaze(generateMazeData(MAZE_SIZE), mazeBlocks, camera, light, _world, _shader);
    timeAccumulator = 0;
}

int Game::Update(float dt)
{
    timeAccumulator += dt;

    while (timeAccumulator >= timeDelta) {
        //FixedUpdate
        glfwPollEvents();

        if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(_window, GL_TRUE);

        camera->damp();
        if (glfwGetKey(_window, GLFW_KEY_UP) == GLFW_PRESS)
            camera->backAndForthMove(1.0, timeDelta);
        if (glfwGetKey(_window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera->backAndForthMove(-1.0, timeDelta);
        if (glfwGetKey(_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera->yaw(1.0, timeDelta);
        if (glfwGetKey(_window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera->yaw(-1.0, timeDelta);
        if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
            camera->yaw(1.0, timeDelta);
        if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
            camera->yaw(-1.0, timeDelta);
        if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
            camera->pitch(1.0, timeDelta);
        if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
            camera->pitch(-1.0, timeDelta);
        if (glfwGetKey(_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (camera->isFreezing())
                camera->verticallyMove(1.0, timeDelta);
        }
        if (glfwGetKey(_window, GLFW_KEY_Z) == GLFW_PRESS)
            terrainDrawMode = terrainDrawMode == GL_FILL ? GL_LINE : GL_FILL;
        if (glfwGetKey(_window, GLFW_KEY_C) == GLFW_PRESS)
            isLightFlashing = !isLightFlashing;
        if (glfwGetKey(_window, GLFW_KEY_R) == GLFW_PRESS)
            winPosition = restart(mazeBlocks, camera, light, _world, _shader);
        
        if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double xPos, yPos; // 0 <= xPos <= SCREEN_W, 0 <= yPos <= SCREEN_H
            glfwGetCursorPos(_window, &xPos, &yPos);
            light->setColor(glm::vec3(1.0, xPos / SCREEN_W, yPos / SCREEN_H));
        }

        _world->stepSimulation(timeDelta, 0);
        timeAccumulator -= timeDelta;
    }    

    if (isLightFlashing)
        light->setColor(abs(glm::cos((float)(glfwGetTime() * glm::radians(10.0f)))) * (float)(rand() % 2 / 2.0 + 0.5) * lightColor);
    
    /*
        Dead condition
    */
    if (camera->getPosition().y <= -5.0) {
        glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
        return -1;
    }

    /*
        Win condition
    */
    if (glm::distance(camera->getPosition(), winPosition) <= 3.0) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        return -1;
    }
    return 1;
}

void Game::Render()
{
    for (Mesh* mesh : mazeBlocks) {
        mesh->Render(mesh->getDynamicsTransform(), light, camera);
    }
    terrain->Render(glm::mat4(1.0f), light, camera);
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)(glfwGetTime() * glm::radians(10.0f)), glm::vec3(0, 1, 0));
    skybox->Render(model, light, camera);
}
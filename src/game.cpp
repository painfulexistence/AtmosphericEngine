#include "game.hpp"
#define MAZE_SIZE 30
#define TILES_TO_REMOVE 600
#define MAZE_ROOFED true
#define TILE_SIZE 5

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

glm::vec3 createMaze(bool** maze, std::vector<Cube*>& cubes, Camera*& camera, Light*& light, btDiscreteDynamicsWorld* world) 
{    
    bool characterPlaced = false;
    glm::vec3 charPos = glm::vec3(0, 0, 0);
    glm::vec3 winPos = glm::vec3(0, 0, 0);
    Cube* tile;

    for (int x = 0; x < MAZE_SIZE; x++) {
        for (int z = 0; z < MAZE_SIZE; z++) {
            if (MAZE_ROOFED) {
                tile = new Cube(TILE_SIZE);
                cubes.push_back(tile);
                tile->AddRigidBody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 3, z - MAZE_SIZE / 2.f), world, 0.f);
            }
            if (maze[x][z]) {
                tile = new Cube(TILE_SIZE);
                cubes.push_back(tile);
                tile->AddRigidBody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 0, z - MAZE_SIZE / 2.f), world, 0.f);
                
                tile = new Cube(TILE_SIZE);
                cubes.push_back(tile);
                tile->AddRigidBody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f), world, 0.f);

                tile = new Cube(TILE_SIZE);
                cubes.push_back(tile);
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
            tile = new Cube(TILE_SIZE);
            cubes.push_back(tile);
            tile->AddRigidBody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, -1, z - MAZE_SIZE / 2.f), world, 0.f);
        }
    }
    camera->setPosition(charPos);
    return winPos;
}

Game::Game(GLFWwindow* window, btDiscreteDynamicsWorld* world)
{
    _window = window;
    _world = world;
}

void Game::Init()
{
    //Terrain material
    {
        std::vector<Shader*> shaders = {
            new Shader("shaders/simple.vert", GL_VERTEX_SHADER),
            new Shader("shaders/glitched_diffuse.frag", GL_FRAGMENT_SHADER)
        };
        Program* program = new Program(shaders);
        _materials.push_back(
            new Material(
                program, (GLuint)4,
                glm::vec3(.25, .20725, .20725),
                glm::vec3(1, .829, .829),
                glm::vec3(.296648, .296648, .296648),
                0.088
            )
        );
    }
    //Skybox material
    {
        std::vector<Shader*> shaders = {
            new Shader("shaders/simple.vert", GL_VERTEX_SHADER),
            new Shader("shaders/glitched_diffuse.frag", GL_FRAGMENT_SHADER)
        };
        Program* program = new Program(shaders);
        
        _materials.push_back(
            new Material(
                program, (GLuint)1,
                glm::vec3(.1, .18725, .1745),
                glm::vec3(.396, .74151, .69102),
                glm::vec3(.992157, .941176, .807843),
                0.1
            )
        );
    }
    //Cube material
    {
        std::vector<Shader*> shaders = {
            new Shader("shaders/instanced.vert", GL_VERTEX_SHADER),
            new Shader("shaders/glitched_diffuse.frag", GL_FRAGMENT_SHADER)
        };
        Program* program = new Program(shaders);
        
        _materials.push_back(
            new Material(
                program, (GLuint)4,
                glm::vec3(0.19225, 0.19225, 0.19225),
                glm::vec3(0.50754, 0.50754, 0.50754),
                glm::vec3(0.508273, 0.508273, 0.508273),
                0.4
            )
        );
    }
    //Green plastic
    {
        std::vector<Shader*> shaders = {
            new Shader("shaders/simple.vert", GL_VERTEX_SHADER),
            new Shader("shaders/glitched_diffuse.frag", GL_FRAGMENT_SHADER)
        };
        Program* program = new Program(shaders);
        _materials.push_back(
            new Material(
                program, (GLuint)3,
                glm::vec3(.0, .0, .0),
                glm::vec3(.5, .0, .0),
                glm::vec3(.7, .6, .6),
                0.25
            )
        );
    }
    camera = new Camera(glm::vec3(0, 2, 0), 0.f, 0.f, glm::radians(45.0f), SCREEN_W / SCREEN_H, _world);
    light = new Light(glm::vec3(10, 50, 10), lightColor);
    terrain = new Terrain(256, 10, new float[100]{0});
    skybox = new Cube(800);
    winPosition = createMaze(generateMazeData(MAZE_SIZE), cubes, camera, light, _world);
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
        if (glfwGetKey(_window, GLFW_KEY_L) == GLFW_PRESS) {
            double xPos, yPos; // 0 <= xPos <= SCREEN_W, 0 <= yPos <= SCREEN_H
            glfwGetCursorPos(_window, &xPos, &yPos);
            light->SetDiffuse(glm::vec3(1.0, xPos / SCREEN_W, yPos / SCREEN_H));
        }
        if (glfwGetKey(_window, GLFW_KEY_F) == GLFW_PRESS)
            isLightFlashing = !isLightFlashing;

        _world->stepSimulation(timeDelta, 0);
        timeAccumulator -= timeDelta;
    }    

    if (isLightFlashing)
        light->SetDiffuse(abs(glm::cos((float)(glfwGetTime() * glm::radians(10.0f)))) * (float)(rand() % 2 / 2.0 + 0.5) * lightColor);
    
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
    // Show a custom window
    {
        int tiles = MAZE_SIZE * MAZE_SIZE + 3 * (MAZE_SIZE * MAZE_SIZE - TILES_TO_REMOVE);

        ImGui::Begin("Game Rendering");
        //ImGui::Text("Draw calls estimate %d", tiles);
        ImGui::Text("Vertices estimate %.1fk", (tiles * 12 + 320 + 24) / 1000.0);
        ImGui::Text("Triangles estimate %.1fk", (tiles * 6 + 200 + 12) / 1000.0);
        ImGui::End();
    }

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 

    glm::vec3 cameraPosition = camera->getPosition();
    glm::mat4 PV = camera->getProjectionViewMatrix();
    glm::vec3 lightPosition = light->GetPosition();  
    glm::vec3 lightDirection = light->GetDirection();
    glm::vec3 lightAmbient = light->GetAmbient();
    glm::vec3 lightDiffuse = light->GetDiffuse();
    glm::vec3 lightSpecular = light->GetSpecular();

    {
        Material* mat = _materials[0];
        Program* prog = mat->GetProgram();
        prog->Activate();

        glm::mat4 worldMatrix = glm::mat4(1.0f);
        glUniformMatrix4fv(prog->GetUniform("World"), 1, GL_FALSE, &worldMatrix[0][0]);
        glUniformMatrix4fv(prog->GetUniform("ProjectionView"), 1, GL_FALSE, &PV[0][0]);
        glUniform3fv(prog->GetUniform("view_pos"), 1, &cameraPosition[0]);
        glUniform3fv(prog->GetUniform("light.position"), 1, &lightPosition[0]);
        glUniform3fv(prog->GetUniform("light.direction"), 1, &lightDirection[0]);
        glUniform3fv(prog->GetUniform("light.ambient"), 1, &lightAmbient[0]); 
        glUniform3fv(prog->GetUniform("light.diffuse"), 1, &lightDiffuse[0]); 
        glUniform3fv(prog->GetUniform("light.specular"), 1, &lightSpecular[0]); 
        glUniform3fv(prog->GetUniform("surf.ambient"), 1, &mat->GetAmbient()[0]);
        glUniform3fv(prog->GetUniform("surf.diffuse"), 1, &mat->GetDiffuse()[0]);
        glUniform3fv(prog->GetUniform("surf.specular"), 1, &mat->GetSpecular()[0]);
        glUniform1f(prog->GetUniform("surf.shininess"), mat->GetShininess());
        glUniform1i(prog->GetUniform("tex"), mat->GetMainTex());
        glUniform1f(prog->GetUniform("time"), (GLfloat)glfwGetTime());
        
        //glStencilMask(0xFF);
        //glStencilFunc(GL_ALWAYS, 1, 0xFF);
        terrain->Render();

        //glStencilMask(0x00);
        //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        //glDepthFunc(GL_ALWAYS);
        //glUniform3fv(prog->GetUniform("surf.ambient"), 1, &glm::vec3(0.9)[0]);
        //terrain->Render(glm::scale(glm::mat4(1.0f), glm::vec3(1.1, 1.1, 1.1)));
        //glDepthFunc(GL_LESS);
        
        //glStencilMask(0xFF);
        //glStencilFunc(GL_ALWAYS, 1, 0xFF);        
    }
    {    
        Material* mat = _materials[2];
        Program* prog = mat->GetProgram();
        prog->Activate();

        glUniformMatrix4fv(prog->GetUniform("ProjectionView"), 1, GL_FALSE, &PV[0][0]);
        glUniform3fv(prog->GetUniform("view_pos"), 1, &cameraPosition[0]);
        glUniform3fv(prog->GetUniform("light.position"), 1, &lightPosition[0]);
        glUniform3fv(prog->GetUniform("light.direction"), 1, &lightDirection[0]);
        glUniform3fv(prog->GetUniform("light.ambient"), 1, &lightAmbient[0]); 
        glUniform3fv(prog->GetUniform("light.diffuse"), 1, &lightDiffuse[0]); 
        glUniform3fv(prog->GetUniform("light.specular"), 1, &lightSpecular[0]);         
        glUniform3fv(prog->GetUniform("surf.ambient"), 1, &mat->GetAmbient()[0]);
        glUniform3fv(prog->GetUniform("surf.diffuse"), 1, &mat->GetDiffuse()[0]);
        glUniform3fv(prog->GetUniform("surf.specular"), 1, &mat->GetSpecular()[0]);
        glUniform1f(prog->GetUniform("surf.shininess"), mat->GetShininess());
        glUniform1i(prog->GetUniform("tex"), mat->GetMainTex());
        glUniform1f(prog->GetUniform("time"), (GLfloat)glfwGetTime());     

        glm::mat4* worldMatrices = new glm::mat4[cubes.size()];
        for (int i = 0; i < cubes.size(); i++) 
        {
            worldMatrices[i] = cubes[i]->getWorldTransform();
        }
        cubes[0]->RenderMultiple(worldMatrices, cubes.size());
    }
    {
        glDisable(GL_CULL_FACE);

        Material* mat = _materials[1];
        Program* prog = mat->GetProgram();
        prog->Activate();

        glm::mat4 worldMatrix = glm::rotate(glm::mat4(1.0f), (float)(glfwGetTime() * glm::radians(10.0f)), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(prog->GetUniform("World"), 1, GL_FALSE, &worldMatrix[0][0]);     
        glUniformMatrix4fv(prog->GetUniform("ProjectionView"), 1, GL_FALSE, &PV[0][0]);
        glUniform3fv(prog->GetUniform("view_pos"), 1, &cameraPosition[0]);
        glUniform3fv(prog->GetUniform("light.position"), 1, &lightPosition[0]);
        glUniform3fv(prog->GetUniform("light.direction"), 1, &lightDirection[0]);
        glUniform3fv(prog->GetUniform("light.ambient"), 1, &lightAmbient[0]); 
        glUniform3fv(prog->GetUniform("light.diffuse"), 1, &lightDiffuse[0]); 
        glUniform3fv(prog->GetUniform("light.specular"), 1, &lightSpecular[0]);         
        glUniform3fv(prog->GetUniform("surf.ambient"), 1, &mat->GetAmbient()[0]);
        glUniform3fv(prog->GetUniform("surf.diffuse"), 1, &mat->GetDiffuse()[0]);
        glUniform3fv(prog->GetUniform("surf.specular"), 1, &mat->GetSpecular()[0]);
        glUniform1f(prog->GetUniform("surf.shininess"), mat->GetShininess());
        glUniform1i(prog->GetUniform("tex"), mat->GetMainTex());
        glUniform1f(prog->GetUniform("time"), (GLfloat)glfwGetTime());     

        skybox->Render();
    }
}
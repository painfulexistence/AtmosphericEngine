#include "game.hpp"

#define MAZE_SIZE 50
#define TILES_TO_REMOVE 2200
#define MAZE_ROOFED false
#define TILE_SIZE 4
#define CHISM_PROBABILITY 10
#define GRAVITY 10
#define AUTO_CLOSE false
#define FIXED_TIME_STEP 1.0 / 60.0

using namespace std;


static vector<vector<bool>> generateMazeData(int size) {
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
    while (tilesConsumed < TILES_TO_REMOVE && tilesConsumed < (size - 2) * (size - 2)) {
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

Game::Game(const shared_ptr<Framework>& framework) : _framework(framework), _program(make_shared<Program>()), _scene(Scene(_program))
{
    btCollisionConfiguration* config = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(config);
    btBroadphaseInterface* broadphase = new btDbvtBroadphase();
    btConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    
    _world = make_shared<btDiscreteDynamicsWorld>(dispatcher, broadphase, solver, config);
    _world->setGravity(btVector3(0, -GRAVITY, 0));
}

Game::~Game() {}

void Game::Init()
{
    _state.timeAccumulator = 0;
    _state.lightColor = glm::vec3(1, 1, 1);
    _state.isLightFlashing = false;
    _state.winPosition = glm::vec3(0, 0, 0);

    _program->Init();
    _scene.Init();
    LoadResources();

    cout << "Game initialized successfully" << endl;
}

void Game::Start()
{
    CameraProperties cProps;
    cProps.aspectRatio = SCREEN_W / (float)SCREEN_H;
    cProps.farClipPlane = 3000.0f;
    camera = new Camera(glm::vec3(0, 2, 0), glm::vec2(0.f, 0.f), cProps);
    camera->Embody(_world);

    LightProperties lProps;
    lProps.diffuse = _state.lightColor;
    light = new Light(glm::vec3(10, 50, 10), lProps, glm::vec3(1, -1, 1));
    
    unique_ptr<Instantiation> rain(new Instantiation(1));
    vector<shared_ptr<Geometry>> spheres(500);
    for (int i = 0; i < spheres.size(); i++)
    {
        shared_ptr<Geometry> s = make_shared<Sphere>((float)(rand() % 9));
        s->Embody(glm::vec3(rand() % 10 - 5, rand() % 100 + 20, rand() % 10 - 5), 1.0f, _world);
        spheres[i] = s;
    }
    rain->Init(spheres);
    _scene.Create(move(rain));

    unique_ptr<Instantiation> terrain(new Instantiation(0));
    auto t = make_shared<Terrain>(MAZE_SIZE * TILE_SIZE, 10, new float[100]{0});
    t->Embody(glm::vec3(0.0f, -15.0f, 0.0f), 0.0f, _world);
    terrain->Init(t);
    _scene.Create(move(terrain));

    unique_ptr<Instantiation> skybox(new Instantiation(1));
    auto b = make_shared<Cube>(800);
    skybox->Init(b);
    _scene.Create(move(skybox));

    unique_ptr<Instantiation> maze(new Instantiation(2));
    vector<shared_ptr<Geometry>> cubes;
    CreateMaze(generateMazeData(MAZE_SIZE), cubes);
    maze->Init(cubes);
    _scene.Create(move(maze));
}

void Game::LoadResources()
{
    // Load textures
    std::vector<std::string> paths = {
        "./resources/beach.png",
        "./resources/starnight.jpg",
        "./resources/grass.png",
        "./resources/brick.jpg",
        "./resources/metal.jpg"
    };
    _framework->Textures(paths);
}

void Game::CreateMaze(const vector<vector<bool>>& maze, vector<shared_ptr<Geometry>>& cubes) 
{    
    bool characterPlaced = false;
    
    shared_ptr<Geometry> tile;
    for (int x = 0; x < MAZE_SIZE; x++) {
        for (int z = 0; z < MAZE_SIZE; z++) {
            if (MAZE_ROOFED) 
            {
                tile = make_shared<Cube>(TILE_SIZE);
                tile->Embody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 3, z - MAZE_SIZE / 2.f), 0.f, _world);
                cubes.push_back(move(tile));
            }
            if (maze[x][z]) 
            {
                for (int h = 0; h < 3; h++)
                {
                    tile = make_shared<Cube>(TILE_SIZE);
                    tile->Embody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, h, z - MAZE_SIZE / 2.f), 0.f, _world);
                    cubes.push_back(move(tile));
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
                    glm::vec3 charPos = (float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f);
                    camera->setPosition(charPos);
                    characterPlaced = true;
                }
                glm::vec3 winPos = (float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 0, z - MAZE_SIZE / 2.f);
                _state.winPosition = winPos;
            }
            tile = make_shared<Cube>(TILE_SIZE);
            tile->Embody((float)TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, -1, z - MAZE_SIZE / 2.f), 0.f, _world);
            cubes.push_back(move(tile));
        }
    }
    cout << "Maze created successfully(length: " << cubes.size() << ")"  << endl;
}

void Game::Update(float dt)
{
    _state.timeAccumulator += dt;

    // Fixed time step update
    while (_state.timeAccumulator >= FIXED_TIME_STEP) {
        _world->stepSimulation(FIXED_TIME_STEP, 0);
        _state.timeAccumulator -= FIXED_TIME_STEP;
    }
    
    // Input handling
    HandleInput();

    // Normal update
    if (_state.isLightFlashing)
        light->SetDiffuse(abs(glm::cos((float)(_framework->GetTime() * glm::radians(10.0f)))) * (float)(rand() % 2 / 2.0 + 0.5) * _state.lightColor);
    if (camera->getPosition().y <= -5.0) {
        //cout << "You lose!" << endl;
        if (AUTO_CLOSE)
        {
            _framework->CloseWindow();
            return;
        }
    }
    if (glm::distance(camera->getPosition(), _state.winPosition) <= 3.0) {
        //cout << "You win!" << endl;
        if (AUTO_CLOSE)
        {
            _framework->CloseWindow();
            return;
        }
    }
    
    float time = (float)_framework->GetTime();
    
    glm::vec3 cameraPosition = camera->getPosition();
    glm::mat4 PV = camera->getProjectionViewMatrix();
    glm::vec3 lightPosition = light->GetPosition();
    glm::vec3 lightDirection = light->GetDirection();
    glm::vec3 lightAmbient = light->GetAmbient();
    glm::vec3 lightDiffuse = light->GetDiffuse();
    glm::vec3 lightSpecular = light->GetSpecular();
    
    glUniformMatrix4fv(_program->GetUniform("ProjectionView"), 1, GL_FALSE, &PV[0][0]);
    glUniform3fv(_program->GetUniform("cam_pos"), 1, &cameraPosition[0]);
    glUniform3fv(_program->GetUniform("light.position"), 1, &lightPosition[0]);
    glUniform3fv(_program->GetUniform("light.direction"), 1, &lightDirection[0]);
    glUniform3fv(_program->GetUniform("light.ambient"), 1, &lightAmbient[0]);
    glUniform3fv(_program->GetUniform("light.diffuse"), 1, &lightDiffuse[0]);
    glUniform3fv(_program->GetUniform("light.specular"), 1, &lightSpecular[0]);
    glUniform1f(_program->GetUniform("time"), (GLfloat)time);
    
    _scene.Update(time);
    
    Render(dt);
    RenderGUI(dt);

    _framework->Swap();    
}

void Game::HandleInput()
{
    _framework->Poll();
    
    camera->damp();

    if (_framework->KeyDown(KEY_UP))
        camera->backAndForthMove(CAMERA_SPEED);

    if (_framework->KeyDown(KEY_DOWN))
        camera->backAndForthMove(-CAMERA_SPEED);

    if (_framework->KeyDown(KEY_RIGHT))
        camera->yaw(CAMERA_ANGULAR_OFFSET);

    if (_framework->KeyDown(KEY_LEFT))
        camera->yaw(-CAMERA_ANGULAR_OFFSET);

    if (_framework->KeyDown(KEY_SPACE)) 
    {
        if (camera->isFreezing()) 
            camera->verticallyMove(CAMERA_VERTICAL_SPEED);
    }

    if (_framework->KeyDown(KEY_D))
        camera->yaw(CAMERA_ANGULAR_OFFSET);

    if (_framework->KeyDown(KEY_A))
        camera->yaw(-CAMERA_ANGULAR_OFFSET);

    if (_framework->KeyDown(KEY_W))
        camera->pitch(CAMERA_ANGULAR_OFFSET);

    if (_framework->KeyDown(KEY_S))
        camera->pitch(-CAMERA_ANGULAR_OFFSET);

    if (_framework->KeyDown(KEY_Z)) 
    {
        glm::vec2 pos = _framework->CursorUV();
        glm::vec3 diff = glm::vec3(1.0, pos.x, pos.y);

        light->SetDiffuse(diff);
    }

    if (_framework->KeyDown(KEY_X))
        _state.isLightFlashing = !_state.isLightFlashing;
}

void Game::Render(float dt)
{
    // Rendering
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //glEnable(GL_STENCIL_TEST);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 
    
    _scene.Render();
}

void Game::RenderGUI(float dt)
{
    // Start Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("General Graphics");
            
    ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
    ImGui::Text("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
    ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
    
    GLint depth, stencil;
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth);    
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil);    
    ImGui::Text("Depth bits: %d", depth);
    ImGui::Text("Stencil bits: %d", stencil);

    GLint maxVertUniforms, maxFragUniforms;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertUniforms);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragUniforms);
    ImGui::Text("Max vertex uniforms: %d bytes", maxVertUniforms / 4);
    ImGui::Text("Max fragment uniforms: %d bytes", maxFragUniforms / 4);

    GLint maxVertUniBlocks, maxFragUniBlocks;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &maxVertUniBlocks);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &maxFragUniBlocks);
    ImGui::Text("Max vertex uniform blocks: %d", maxVertUniBlocks);
    ImGui::Text("Max fragment uniform blocks: %d", maxFragUniBlocks);

    GLint maxElementIndices, maxElementVertices;
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxElementIndices);
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxElementVertices);
    ImGui::Text("Max element indices: %d", maxElementIndices);
    ImGui::Text("Max element vertices: %d", maxElementVertices);

    ImGui::ColorEdit3("Clear color", (float*)&clearColor);
    ImGui::Text("Draw time: %.3f s/frame", dt);
    ImGui::Text("Frame rate: %.1f FPS", 1.0f / dt);
    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    ImGui::Begin("Game Rendering");

    int tiles = MAZE_SIZE * MAZE_SIZE + 3 * (MAZE_SIZE * MAZE_SIZE - TILES_TO_REMOVE);
    ImGui::Text("Vertices estimate %.1fk", (tiles * 12 + 320 + 24) / 1000.0);
    ImGui::Text("Triangles estimate %.1fk", (tiles * 6 + 200 + 12) / 1000.0);
    
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

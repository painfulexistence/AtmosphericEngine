#include "game.hpp"

#define MAZE_SIZE 50
#define TILES_TO_REMOVE 2200
#define MAZE_ROOFED false
#define TILE_SIZE 4
#define CHISM_PROBABILITY 10
#define GRAVITY 10
#define AUTO_CLOSE false
#define CHECK_FRAMEWORK_ERRORS false

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

Game::Game(const shared_ptr<Framework>& framework) : _framework(framework), _scene(Scene())
{
    btCollisionConfiguration* config = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(config);
    btBroadphaseInterface* broadphase = new btDbvtBroadphase();
    btConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    
    _world = make_shared<btDiscreteDynamicsWorld>(dispatcher, broadphase, solver, config);
    _world->setGravity(btVector3(0, -GRAVITY, 0));

    std::vector<Shader> colorShaders = {
        Shader("./resources/shaders/multilight_shadow.vert", GL_VERTEX_SHADER),
        Shader("./resources/shaders/multilight_shadow.frag", GL_FRAGMENT_SHADER)
    };
    std::vector<Shader> depthShaders = {
        Shader("./resources/shaders/depth_simple.vert", GL_VERTEX_SHADER),
        Shader("./resources/shaders/depth_simple.frag", GL_FRAGMENT_SHADER)
    };
    colorProgram = Program(colorShaders);
    depthProgram = Program(depthShaders);

    // Setup render pipeline
    colorProgram.Activate();

    _state.timeAccumulator = 0;
    _state.lightColor = glm::vec3(1.0, 0.067, 1.0);
    _state.isLightFlashing = false;
    _state.winPosition = glm::vec3(0, 0, 0);

    cout << "Game initialized successfully" << endl;
}

Game::~Game() {}

void Game::Run()
{
    CameraProperties cProps;
    cProps.origin = glm::vec3(0, 2, 0);
    cProps.aspectRatio = SCREEN_W / (float)SCREEN_H;
    cProps.farClipPlane = 3000.0f;
    camera = Camera(cProps);
    camera.Embody(_world);

    LightProperties lProps;
    
    lProps.intensity = 0.5;
    lProps.direction = glm::vec3(-0.21, -0.72, -0.5);
    lProps.diffuse = _state.lightColor;
    mainLight = Light(lProps, DIR_LIGHT);
    
    lProps.intensity = 4.0;
    lProps.position = glm::vec3(-rand() % 50, 20, -rand() % 50);
    lProps.diffuse = glm::vec3(1, 1, 0);
    auxLights.push_back(Light(lProps, POINT_LIGHT));    
    lProps.position = glm::vec3(-rand() % 50, 20, -rand() % 50);
    auxLights.push_back(Light(lProps, POINT_LIGHT));

    lProps.intensity = 4.0;
    lProps.position = glm::vec3(rand() % 50, 20, rand() % 50);
    lProps.diffuse = glm::vec3(1, 0, 1);
    auxLights.push_back(Light(lProps, POINT_LIGHT));
    lProps.position = glm::vec3(rand() % 50, 20, rand() % 50);
    auxLights.push_back(Light(lProps, POINT_LIGHT));
    
    lProps.intensity = 4.0;
    lProps.position = glm::vec3(rand() % 50, 20, -rand() % 50);
    lProps.diffuse = glm::vec3(0, 1, 1);
    auxLights.push_back(Light(lProps, POINT_LIGHT));    
    lProps.position = glm::vec3(rand() % 50, 20, -rand() % 50);
    auxLights.push_back(Light(lProps, POINT_LIGHT));

    unique_ptr<Instantiation> rain(new Instantiation(1));
    vector<shared_ptr<Geometry>> spheres(100);
    for (int i = 0; i < spheres.size(); i++)
    {
        shared_ptr<Geometry> s = make_shared<Sphere>(5.0f);
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

    double pastTime = _framework->GetTime();
    while(!_framework->IsWindowOpen())
    {
        double currentTime = _framework->GetTime();
        Update(float(currentTime - pastTime));
        pastTime = currentTime;
        #if CHECK_FRAMEWORK_ERRORS
        _framework->CheckErrors();
        #endif
    }
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
                    camera.SetOrigin(charPos);
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
        mainLight.SetDiffuse(abs(glm::cos((float)(_framework->GetTime() * glm::radians(10.0f)))) * (float)(rand() % 2 / 2.0 + 0.5) * _state.lightColor);
    if (camera.GetOrigin().y <= -5.0) {
        //cout << "You lose!" << endl;
        if (AUTO_CLOSE)
        {
            _framework->CloseWindow();
            return;
        }
    }
    if (glm::distance(camera.GetOrigin(), _state.winPosition) <= 3.0) {
        //cout << "You win!" << endl;
        if (AUTO_CLOSE)
        {
            _framework->CloseWindow();
            return;
        }
    }
    
    colorProgram.SetUniform(string("ProjectionView"), camera.getProjectionViewMatrix());
    colorProgram.SetUniform(string("cam_pos"), camera.GetOrigin());

    colorProgram.SetUniform(string("main_light.direction"), mainLight.GetDirection());
    colorProgram.SetUniform(string("main_light.ambient"), mainLight.GetAmbient());
    colorProgram.SetUniform(string("main_light.diffuse"), mainLight.GetDiffuse());
    colorProgram.SetUniform(string("main_light.specular"), mainLight.GetSpecular());
    colorProgram.SetUniform(string("main_light.intensity"), mainLight.GetIntensity());

    int auxLightCount = min((int)auxLights.size(), MAX_NUM_AUX_LIGHTS);
    for (int i = 0; i < auxLightCount; ++i)
    {
        Light& l = auxLights[i];
        colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].position"), l.GetPosition());
        colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].ambient"), l.GetAmbient());
        colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].diffuse"), l.GetDiffuse());
        colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].specular"), l.GetSpecular());
        colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].attenuation"), l.GetAttenuation());
        colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].intensity"), l.GetIntensity());
    }
    colorProgram.SetUniform(string("aux_light_count"), auxLightCount);
    
    float time = (float)_framework->GetTime();
    colorProgram.SetUniform(string("time"), time);
    
    _scene.Update(time);
    
    Render(dt);
    RenderGUI(dt);

    _framework->SwapBuffers();    
}

void Game::HandleInput()
{
    _framework->PollEvents();
    
    camera.damp();

    if (_framework->IsKeyDown(KEY_W))
        camera.backAndForthMove(CAMERA_SPEED);

    if (_framework->IsKeyDown(KEY_S))
    {
        camera.backAndForthMove(-CAMERA_SPEED);
    }
    if (_framework->IsKeyDown(KEY_D))
    {
        camera.horizontallyMove(CAMERA_SPEED);
        camera.yaw(0.3 * CAMERA_ANGULAR_OFFSET);
    }
    if (_framework->IsKeyDown(KEY_A))
    {
        camera.horizontallyMove(-CAMERA_SPEED);
        camera.yaw(-0.3 * CAMERA_ANGULAR_OFFSET);
    }
    if (_framework->IsKeyDown(KEY_UP))
    {
        camera.pitch(CAMERA_ANGULAR_OFFSET);
    }
    if (_framework->IsKeyDown(KEY_DOWN))
    {
        camera.pitch(-CAMERA_ANGULAR_OFFSET);
    }
    if (_framework->IsKeyDown(KEY_RIGHT))
    {
        camera.yaw(CAMERA_ANGULAR_OFFSET);
    }
    if (_framework->IsKeyDown(KEY_LEFT))
    {
        camera.yaw(-CAMERA_ANGULAR_OFFSET);
    }
    if (_framework->IsKeyDown(KEY_SPACE)) 
    {
        if (camera.isFreezing()) 
            camera.verticallyMove(CAMERA_VERTICAL_SPEED);
    }
    if (_framework->IsKeyDown(KEY_Z)) 
    {
        glm::vec2 pos = _framework->GetCursorUV();
        glm::vec3 diff = glm::vec3(1.0, pos.x, pos.y);

        mainLight.SetDiffuse(diff);
    }
    if (_framework->IsKeyDown(KEY_X))
    {
        _state.isLightFlashing = !_state.isLightFlashing;
    }
}

void Game::Render(float dt)
{
    // 0. Preparing
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //glEnable(GL_STENCIL_TEST);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //glStencilFunc(GL_ALWAYS, 1, 0xFF);

    // 1. Depth Pass
    glViewport(0, 0, SHADOW_W, SHADOW_H);
    glBindFramebuffer(GL_FRAMEBUFFER, _framework->GetShadowFramebuffer());
    glClear(GL_DEPTH_BUFFER_BIT);
    depthProgram.Activate();
    depthProgram.SetUniform(string("LightProjectionView"), mainLight.GetProjectionViewMatrix());
    _scene.Render(depthProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. Color Pass
    glViewport(0, 0, SCREEN_W, SCREEN_H);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _framework->GetShadowMap());
    const vector<GLuint>& textures = _framework->GetTextures();
    for (int i = 0; i < textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + NUM_MAP_TEXS + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }

    colorProgram.Activate();
    colorProgram.SetUniform(string("LightProjectionView"), mainLight.GetProjectionViewMatrix());
    colorProgram.SetUniform(string("shadow_map_unit"), (int)0);
    _scene.Render(colorProgram);
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
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

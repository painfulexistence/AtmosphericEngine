#include "game.hpp"

using namespace std;

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

static glm::mat4 ConvertPhysicalMatrix(const btTransform& trans)
{
    btScalar mat[16] = {0.0f};
    trans.getOpenGLMatrix(mat);
        
    return glm::mat4(
        mat[0], mat[1], mat[2], mat[3],
        mat[4], mat[5], mat[6], mat[7],
        mat[8], mat[9], mat[10], mat[11],
        mat[12], mat[13], mat[14], mat[15]
    );
}

Game::Game(Framework& framework) : framework(framework), entities(Entity::Entities) {}

void Game::Run()
{
    Lua::Lib();
    Lua::Source("./resources/scripts/config.lua");
    Lua::Source("./resources/scripts/main.lua");
    Lua::Run("init()");
    Lua::L.set_function("get_cursor_uv", &Framework::GetCursorUV, framework);
    Lua::L.set_function("swap_buffers", &Framework::SwapBuffers, framework);
    Lua::L.set_function("check_errors", &Framework::CheckErrors, framework);
    Lua::L.set_function("get_time", &Framework::GetTime, framework);
    Lua::L.set_function("poll_events", &Framework::PollEvents, framework);
    Lua::L.set_function("is_window_open", &Framework::IsWindowOpen, framework);
    Lua::L.set_function("is_key_down", &Framework::IsKeyDown, framework);
    Lua::L.set_function("close_window", &Framework::CloseWindow, framework);

    // Create shader programs
    const auto& t = Lua::L["shaders"];
    vector<Shader> colorShaders = {
        Shader(string(t["color"]["vert"]), GL_VERTEX_SHADER),
        Shader(string(t["color"]["frag"]), GL_FRAGMENT_SHADER)
    };
    vector<Shader> depthShaders = {
        Shader(string(t["depth"]["vert"]), GL_VERTEX_SHADER),
        Shader(string(t["depth"]["frag"]), GL_FRAGMENT_SHADER)
    };
    colorProgram = Program(colorShaders);
    depthProgram = Program(depthShaders);

    // Create camera 
    CameraProperties cProps;
    cProps.origin = glm::vec3(0, 2, 0);
    cProps.aspectRatio = SCREEN_W / (float)SCREEN_H;
    cProps.farClipPlane = 3000.0f;
    camera = Camera(cProps);
    camera.SetGraphicsId(scene.CreateGhostGeometry());
    camera.SetPhysicsId(world.CreateCapsuleImpostor(btVector3(0.f, 2.f, 0.f), 1.0f, 4.0f, 10.0f));
    world.SetImpostorLinearFactor(camera.GetPhysicsId(), btVector3(1, 1, 1));
    world.SetImpostorAngularFactor(camera.GetPhysicsId(), btVector3(0, 0, 0));
    entities.push_back(camera);

    // Create lights
    LightProperties lProps;

    lProps.intensity = 0.5;
    lProps.direction = glm::vec3(-0.21, -0.72, -0.5);
    lProps.diffuse = glm::vec3(Lua::L["game_state"]["light_color"][1], Lua::L["game_state"]["light_color"][2], Lua::L["game_state"]["light_color"][3]);
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
    
    {
        auto mesh = make_shared<Mesh>();
        mesh->AsCube(800.0f);
        mesh->SetMaterial(Material::Night);
        mesh->SetCullFaceMode(GL_FRONT);
        Scene::MeshTable.insert({"Skybox", mesh});
        
        auto ent = Entity();
        ent.SetGraphicsId(scene.CreateMeshGeometry("Skybox"));
        entities.push_back(ent);
    }
    {
        auto mesh = make_shared<Mesh>();
        mesh->AsSphere();
        mesh->SetMaterial(Material::GreenPlastic);
        Scene::MeshTable.insert({"Sphere", mesh});

        for (int i = 0; i < 50; ++i)
        {
            float diameter = (float)(rand() % 20 + 5);

            auto ent = Entity();
            ent.SetGraphicsId(scene.CreateMeshGeometry("Sphere", glm::scale(glm::mat4(1.0), glm::vec3(diameter))));
            ent.SetPhysicsId(world.CreateSphereImpostor(btVector3(rand() % 10 - 5, rand() % 100 + 20, rand() % 10 - 5), diameter, 1.0f));
            entities.push_back(ent);
        }
    }
    CreateMaze();
    Lua::Run("load()");

    double pastTime = framework.GetTime();
    while(!framework.IsWindowOpen())
    {
        double currentTime = framework.GetTime();
        float deltaTime = (float)currentTime - (float)pastTime;
        pastTime = currentTime;

        Update(deltaTime, (float)currentTime);

        Render(deltaTime);
    }
}

void Game::CreateMaze() 
{   
    const auto& t = Lua::L["maze"];
    const int MAZE_SIZE = t["size"];
    const float TILE_SIZE = t["tile_size"];
    const bool MAZE_ROOFED = t["roofed"];
    const int TILES_TO_REMOVE = t["tiles_to_remove"];
    const float CHISM_PROBABILITY = t["chism_probability"];
    const auto& WIN_COORD = t["win_coord"];

    {
        auto mesh = make_shared<Mesh>();
        mesh->AsCube((float)TILE_SIZE);
        mesh->SetMaterial(Material::Ivory);
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
                        scene.SetGeometryModelWorldTransform(camera.GetGraphicsId(), glm::translate(glm::mat4(1.0f), TILE_SIZE * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f)));
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
        mesh->SetMaterial(Material::Pearl);
        Scene::MeshTable.insert({"Terrain", mesh});

        auto ent = Entity();
        ent.SetGraphicsId(scene.CreateMeshGeometry("Terrain"));
        ent.SetPhysicsId(world.CreateBoxImpostor(btVector3(0.0f, -10.0f, 0.0f), btVector3(size, 0.2f, size)));
        entities.push_back(ent);
    }
}

void Game::Update(float dt, float time)
{    
    world.Update(dt);
    world.DampenImpostor(camera.GetPhysicsId());
    for (const auto& ent : Entity::WithPhysicsComponent())
    {
        const glm::mat4& m2w = ConvertPhysicalMatrix(world.GetImpostorTransform(ent.GetPhysicsId()));
        scene.SetGeometryModelWorldTransform(ent.GetGraphicsId(), m2w);
    }

    glm::mat4 cameraTransform = scene.GetGeometryWorldMatrix(camera.GetGraphicsId());
    glm::vec3 eyePos = camera.GetEye(cameraTransform);

    colorProgram.Activate();
    colorProgram.SetUniform(string("cam_pos"), eyePos);
    colorProgram.SetUniform(string("ProjectionView"), camera.GetProjectionMatrix() * camera.GetViewMatrix(cameraTransform));
    colorProgram.SetUniform(string("time"), time);

    if ((bool)Lua::L["game_state"]["is_light_flashing"])
    {
        glm::vec3 col = glm::vec3(Lua::L["game_state"]["light_color"][1], Lua::L["game_state"]["light_color"][2], Lua::L["game_state"]["light_color"][3]);
        mainLight.SetDiffuse(abs(glm::cos(glm::radians(10.0f) * time)) * (float)(rand() % 2 / 2.0 + 0.5) * col);
    }
    if (eyePos.y <= -5.0) 
    {
        Lua::Run("on_game_over()");
    }
    if (glm::distance(glm::vec2(eyePos.x, eyePos.z), glm::vec2(Lua::L["maze"]["win_coord"][1], Lua::L["maze"]["win_coord"][3])) <= 5.0) 
    {
        Lua::Run("on_complete()");
    }

    HandleInput();
}

void Game::HandleInput()
{
    const std::uint64_t& impostor = camera.GetPhysicsId();

    btVector3 currentVel;    
    if (!world.GetImpostorLinearVelocity(impostor, currentVel)) 
        throw runtime_error("Impostor not found");

    framework.PollEvents();

    if (framework.IsKeyDown(KEY_W))
    {
        glm::vec3 v = camera.CreateLinearVelocity(Axis::FRONT);
        world.SetImpostorLinearVelocity(impostor, btVector3(v.x, currentVel.y(), v.z));
    }
    if (framework.IsKeyDown(KEY_S))
    {
        glm::vec3 v = camera.CreateLinearVelocity(Axis::FRONT);
        world.SetImpostorLinearVelocity(impostor, btVector3(-v.x, currentVel.y(), -v.z));
    }
    if (framework.IsKeyDown(KEY_D))
    {
        camera.yaw(0.3 * CAMERA_ANGULAR_OFFSET);
        glm::vec3 v = camera.CreateLinearVelocity(Axis::RIGHT);
        world.SetImpostorLinearVelocity(impostor, btVector3(v.x, currentVel.y(), v.z));
    }
    if (framework.IsKeyDown(KEY_A))
    {
        camera.yaw(-0.3 * CAMERA_ANGULAR_OFFSET);
        glm::vec3 v = camera.CreateLinearVelocity(Axis::RIGHT);
        world.SetImpostorLinearVelocity(impostor, btVector3(-v.x, currentVel.y(), -v.z));
    }
    if (framework.IsKeyDown(KEY_UP))
    {
        camera.pitch(CAMERA_ANGULAR_OFFSET);
    }
    if (framework.IsKeyDown(KEY_DOWN))
    {
        camera.pitch(-CAMERA_ANGULAR_OFFSET);
    }
    if (framework.IsKeyDown(KEY_RIGHT))
    {
        camera.yaw(CAMERA_ANGULAR_OFFSET);
    }
    if (framework.IsKeyDown(KEY_LEFT))
    {
        camera.yaw(-CAMERA_ANGULAR_OFFSET);
    }
    if (framework.IsKeyDown(KEY_SPACE)) 
    {
        world.GetImpostorLinearVelocity(impostor, currentVel); // update velcoity to reflect current horizontal speed
        glm::vec3 v = camera.CreateLinearVelocity(Axis::UP);
        world.SetImpostorLinearVelocity(impostor, btVector3(currentVel.x(), v.y, currentVel.z()));
    }
    if (framework.IsKeyDown(KEY_Z)) 
    {
        glm::vec2 pos = framework.GetCursorUV();
        glm::vec3 diff = glm::vec3(1.0, pos.x, pos.y);

        mainLight.SetDiffuse(diff);
    }
    if (framework.IsKeyDown(KEY_X))
    {
        Lua::L["game_state"]["is_light_flashing"] = !(bool)Lua::L["game_state"]["is_light_flashing"];
    }
}

void Game::Render(float dt)
{
    // 1. Depth Pass
    glViewport(0, 0, SHADOW_W, SHADOW_H);
    glBindFramebuffer(GL_FRAMEBUFFER, framework.GetShadowFramebuffer());
    glClear(GL_DEPTH_BUFFER_BIT);
    depthProgram.Activate();
    depthProgram.SetUniform(string("LightProjectionView"), mainLight.GetProjectionViewMatrix());
    scene.Render(depthProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. Color Pass
    glViewport(0, 0, SCREEN_W, SCREEN_H);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framework.GetShadowMap());
    const vector<GLuint>& textures = framework.GetTextures();
    for (int i = 0; i < textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + NUM_MAP_TEXS + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }
    colorProgram.Activate();
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
    colorProgram.SetUniform(string("LightProjectionView"), mainLight.GetProjectionViewMatrix());
    colorProgram.SetUniform(string("shadow_map_unit"), (int)0);
    scene.Render(colorProgram);

    RenderGUI(dt);

    Lua::L["dt"] = dt;
    Lua::Run("draw(dt)");    
}

void Game::RenderGUI(float dt)
{
    // Start Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("General Graphics");
    {
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
    }
    ImGui::End();

    ImGui::Begin("Realtime Rendering");
    {
        ImGui::ColorEdit3("Clear color", (float*)&clearColor);
        ImGui::Text("Entities count: %lu", entities.size());
        ImGui::Text("Draw time: %.3f s/frame", dt);
        ImGui::Text("Frame rate: %.1f FPS", 1.0f / dt);
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

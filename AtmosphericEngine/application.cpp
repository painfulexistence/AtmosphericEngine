#include "application.hpp"
#include "window.hpp"
#include "game_object.hpp"
#include "scene.hpp"
#include "impostor.hpp"

using namespace std;

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

Application::Application()
{
    Log("Launching...");
    _window = std::make_shared<Window>();; // Multi-window not supported now
    _window->Init();
    _window->InitImGui();
}

Application::~Application()
{
    Log("Exiting...");
    _window->DeinitImGui();
    for (const auto& go : _entities)
        delete go;
}

void Application::Run()
{
    Log("Initializing subsystems...");

    console.Init(this);
    input.Init(this);
    audio.Init(this);
    graphics.Init(this);
    physics.Init(this); // Note that physics debug drawer is dependent on graphics server
    script.Init(this);
    for (auto& subsystem : _subsystems) {
        subsystem->Init(this);
    }
    this->_initialized = true;

    Log("Subsystems initialized.");

    OnLoad();

    float lastTime = GetWindowTime();
    float deltaTime = 0;
    while (!_window->IsClosing())
    {
        _window->PollEvents();

        float currentTime = GetWindowTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        FrameData currentFrame = { GetClock(), GetWindowTime(), deltaTime };
#if SINGLE_THREAD
        Update(currentFrame);
        SyncTransformWithPhysics();
        Render(currentFrame);
        PresentWindow(currentFrame);
#else
        std::thread fork(&Application::Process, this, currentFrame);
        Render(currentFrame);
        PresentWindow(currentFrame);
        fork.join();
        SyncTransformWithPhysics();
#endif

        Tick();
    }
    Log("Game quitted.");
}

void Application::LoadScene(SceneDef& scene)
{
    Log("Loading scene...");

    graphics.LoadTextures(scene.textures);
    script.Print("Textures created.");

    graphics.LoadColorShader(ShaderProgram(scene.shaders["color"]));
    graphics.LoadDebugShader(ShaderProgram(scene.shaders["debug_line"]));
    graphics.LoadDepthShader(ShaderProgram(scene.shaders["depth"]));
    graphics.LoadDepthCubemapShader(ShaderProgram(scene.shaders["depth_cubemap"]));
    graphics.LoadTerrainShader(ShaderProgram(scene.shaders["terrain"]));
    graphics.LoadPostProcessShader(ShaderProgram(scene.shaders["hdr"]));
    script.Print("Shaders created.");

    for (const auto& mat : scene.materials) {
        graphics.materials.push_back(new Material(mat));
    }
    script.Print("Materials created.");

    for (const auto& go : scene.gameObjects) {
        auto entity = CreateGameObject();
        if (go.camera.has_value()) {
            entity->AddCamera(go.camera.value());
        }
        if (go.light.has_value()) {
            Log(fmt::format("Adding light to {}", go.name));
            entity->AddLight(go.light.value());
        }
    }
    script.Print("Game objects created.");

    mainCamera = graphics.GetMainCamera();
    mainLight = graphics.GetMainLight();
}

void Application::Quit()
{
    Log("Requested to quit.");
    _window->Close();
}

void Application::Update(const FrameData& props)
{
    float dt = props.deltaTime;

    OnUpdate(dt, GetWindowTime());

    //ecs.Process(dt); // Note that most of the entity manipulation logic should be put there
    console.Process(dt);
    input.Process(dt);
    audio.Process(dt);
    script.Process(dt);
    physics.Process(dt); // TODO: Update only every entity's physics transform
    for (auto& subsystem : _subsystems) {
        subsystem->Process(dt);
    }

    #if SHOW_PROCESS_COST
    Log(fmt::format("Update costs {} ms", (GetWindowTime() - time) * 1000));
    #endif
}

void Application::Render(const FrameData& props)
{
    float dt = props.deltaTime;
    float time = GetWindowTime();

    // Note that draw calls are asynchronous, which means they return immediately.
    // So the drawing time can only be calculated along with the image presenting.
    graphics.Process(dt); // TODO: Generate command buffers according to entity transforms
    // graphics.Render(dt);
    // Nevertheless, glFinish() can force the GPU process all the commands synchronously.
    // glFinish();

    _window->BeginImGuiFrame();

    ImGui::Begin("System Information");
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

    ImGui::Begin("Engine Subsystems");
    {
        graphics.DrawImGui(dt);
        for (auto subsystem : _subsystems) {
            subsystem->DrawImGui(dt);
        }
    }
    ImGui::End();

    _window->EndImGuiFrame();

    #if SHOW_RENDER_AND_DRAW_COST
    Log(fmt::format("Render & draw cost {} ms", (GetWindowTime() - time) * 1000));
    #endif
}

void Application::PresentWindow(const FrameData& props)
{
    float time = GetWindowTime();

    this->_window->Present();

    #if SHOW_VSYNC_COST
    Log(fmt::format("Vsync cost {} ms", (Time() - time) * 1000));
    #endif
}

void Application::SyncTransformWithPhysics()
{
    float time = GetWindowTime();

    //ecs.SyncTransformWithPhysics();
    for (auto go : _entities)
    {
        auto impostor = dynamic_cast<Impostor*>(go->GetComponent("Physics"));
        if (impostor == nullptr)
            continue;
        go->SyncObjectTransform(impostor->GetWorldTransform());
    }

    #if SHOW_SYNC_COST
    Log(fmt::format("Sync cost {} ms", (Time() - time) * 1000));
    #endif
}

void Application::Log(std::string message)
{
    #if RUNTIME_LOG_ON
        fmt::print("[Engine] {}\n", message);
    #endif
}

void Application::Tick()
{
    this->_clock++;
}

uint64_t Application::GetClock()
{
    return this->_clock;
}

std::shared_ptr<Window> Application::GetWindow()
{
    return this->_window;
}

float Application::GetWindowTime()
{
    return this->_window->GetTime();
}

void Application::SetWindowTime(float time)
{
    this->_window->SetTime(time);
}

std::string Application::GetWindowTitle()
{
    return this->_window->GetTitle();
}

void Application::SetWindowTitle(const std::string& title)
{
    this->_window->SetTitle(title);
}

GameObject* Application::CreateGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
    auto e = new GameObject(&graphics, &physics, position, rotation, scale);
    _entities.push_back(e);
    return e;
}
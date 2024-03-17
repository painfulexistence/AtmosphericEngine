#include "Application.hpp"
//#include <iostream> // Note that IO should only be used for debugging here
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
    Window* win = new Window();
    win->Init();
    this->_window = win; // Multi-window not supported now
}

Application::~Application()
{
    Log("Exiting...");
    for (const auto& go : gameObjects)
        delete go;
    for (const auto& [name, model] : Model::ModelList)
        delete model;
    delete this->_mb;
    delete this->_window;
}

void Application::Run()
{
    Log("Initializing...");
    console.Init(_mb, this);
    gui.Init(_mb, this);
    input.Init(_mb, this);
    physics.Init(_mb, this);
    graphics.Init(_mb, this);
    script.Init(_mb, this);
    //ecs.Init(_mb, this);
    this->_initialized = true;
    
    Log("Loading data...");
    Load();
    
    float lastFrameTime = GetWindowTime();
    float deltaTime = 0;
    while (!this->_quitted)
    {
        Tick();
        this->_window->PollEvents();
        if (this->_window->IsClosing())
            this->_mb->PostMessage(MessageType::ON_QUIT);

        float currentFrameTime = GetWindowTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        FrameProps currentFrame = FrameProps(this->GetClock(), GetWindowTime(), deltaTime);

        #if SINGLE_THREAD
        #pragma region main_loop_single_thread
        Process(currentFrame);
        SyncTransformWithPhysics();
        Render(currentFrame);
        Draw(currentFrame);
        #pragma endregion
        #else
        #pragma region main_loop_double_thread
        std::thread fork(&Application::Process, this, currentFrame);
        Render(currentFrame);
        Draw(currentFrame);
        fork.join();
        SyncTransformWithPhysics();
        #pragma endregion
        #endif
    }
    Log("Game quitted.");
}

void Application::Quit()
{
    Log("Requested to quit.");
    this->_quitted = true;
}

void Application::Process(const FrameProps& props)
{
    float dt = props.deltaTime;
    float time = GetWindowTime();
    
    Update(dt, time);
    //ecs.Process(dt); // Note that most of the entity manipulation logic should be put there
    BroadcastMessages();
    console.Process(dt);
    input.Process(dt);
    script.Process(dt);
    physics.Process(dt); // TODO: Update only every entity's physics transform
    graphics.Process(dt); // TODO: Generate command buffers according to entity transforms

    #if SHOW_PROCESS_COST
    Log(fmt::format("Update costs {} ms", (Time() - time) * 1000));
    #endif
}

void Application::Render(const FrameProps& props)
{
    float dt = props.deltaTime;
    float time = GetWindowTime();

    // Note that draw calls are asynchronous, which means they return immediately. 
    // So the drawing time can only be calculated along with the image presenting.
    graphics.Render(dt); 
    gui.Render(dt);
    // Nevertheless, glFinish() can force the GPU process all the commands synchronously.
    glFinish();

    #if SHOW_RENDER_AND_DRAW_COST
    Log(fmt::format("Render & draw cost {} ms", (GetWindowTime() - time) * 1000));
    #endif
}

void Application::Draw(const FrameProps& props)
{
    float time = GetWindowTime();

    this->_window->SwapBuffers();

    #if SHOW_VSYNC_COST
    Log(fmt::format("Vsync cost {} ms", (Time() - time) * 1000));
    #endif
}

void Application::BroadcastMessages()
{
    this->_mb->Process();
}

void Application::SyncTransformWithPhysics()
{
    float time = GetWindowTime();

    //ecs.SyncTransformWithPhysics();
    for (auto go : gameObjects)
    {
        auto impostor = dynamic_cast<Impostor*>(go->GetComponent("Physics"));
        if (impostor == nullptr)
            continue;
        go->SetModelWorldTransform(impostor->GetCenterOfMassWorldTransform());
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

float Application::GetWindowTime()
{
    return this->_window->GetTime();
}

Window* Application::GetWindow()
{
    return this->_window;
}
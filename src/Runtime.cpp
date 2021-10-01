#include "Runtime.hpp"
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

Runtime::Runtime()
{
    Log("Launching...");
    this->_win = this->_app->GetActiveWindow(); // Multi-window not supported now
}

Runtime::~Runtime()
{
    Log("Exiting...");
    for (const auto& go : gameObjects)
        delete go;
    for (const auto& [name, model] : Model::ModelList)
        delete model;
    delete this->_mb;
    delete this->_app;
}

void Runtime::Execute()
{
    Log("Initializing...");
    console.Init(_mb, _app);
    gui.Init(_mb, _app);
    input.Init(_mb, _app);
    physics.Init(_mb, _app);
    graphics.Init(_mb, _app);
    script.Init(_mb, _app);
    //ecs.Init(_mb, _app);
    this->_initialized = true;
    
    Log("Loading data...");
    Load();
    
    float lastFrameTime = Time();
    float deltaTime = 0;
    while (!this->_quitted)
    {
        this->_app->Tick();
        this->_win->PollEvents();
        if (this->_win->IsClosing())
            this->_mb->PostMessage(MessageType::ON_QUIT);

        float currentFrameTime = Time();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        FrameProps currentFrame = FrameProps(this->_app->GetClock(), Time(), deltaTime);

        #if SINGLE_THREAD
        #pragma region main_loop_single_thread
        Process(currentFrame);
        SyncTransformWithPhysics();
        Render(currentFrame);
        Draw(currentFrame);
        #pragma endregion
        #else
        #pragma region main_loop_double_thread
        std::thread fork(&Runtime::Process, this, currentFrame);
        Render(currentFrame);
        Draw(currentFrame);
        fork.join();
        SyncTransformWithPhysics();
        #pragma endregion
        #endif
    }
    Log("Game quitted.");
}

void Runtime::Quit()
{
    Log("Requested to quit.");
    this->_quitted = true;
}

void Runtime::Process(const FrameProps& props)
{
    float dt = props.deltaTime;
    float time = Time();
    
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

void Runtime::Render(const FrameProps& props)
{
    float dt = props.deltaTime;
    float time = Time();

    // Note that draw calls are asynchronous, which means they return immediately. 
    // So the drawing time can only be calculated along with the image presenting.
    graphics.Render(dt); 
    gui.Render(dt);
    // Nevertheless, glFinish() can force the GPU process all the commands synchronously.
    glFinish();

    #if SHOW_RENDER_AND_DRAW_COST
    Log(fmt::format("Render & draw cost {} ms", (Time() - time) * 1000));
    #endif
}

void Runtime::Draw(const FrameProps& props)
{
    float time = Time();

    this->_win->SwapBuffers();

    #if SHOW_VSYNC_COST
    Log(fmt::format("Vsync cost {} ms", (Time() - time) * 1000));
    #endif
}

void Runtime::BroadcastMessages()
{
    this->_mb->Process();
}

void Runtime::SyncTransformWithPhysics()
{
    float time = Time();

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

float Runtime::Time()
{
    return this->_app->GetTime();
}

void Runtime::Log(std::string message)
{
    #if RUNTIME_LOG_ON
        fmt::print("[Engine] {}\n", message);
    #endif
}
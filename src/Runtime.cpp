#include "Runtime.hpp"
using namespace std;

Runtime::Runtime() : entities(Entity::Entities)
{
    cout << "[Engine] Launching..." << endl;
}

Runtime::~Runtime()
{
    cout << "[Engine] Exiting..." << endl;
    delete _mb;
    delete _fw;
}
    
void Runtime::Init()
{
    cout << "[Engine] Initializing..." << endl;
    //setbuf(stdout, NULL); //Cancel output stream buffering so that output can be seen immediately
    srand(time(NULL)); //Dont's use glfwGetTime() bc it only starts to calculate time since window was created
    console.Init(_mb, _fw);
    input.Init(_mb, _fw);
    world.Init(_mb, _fw);
    //renderer.Init(_mb, _fw);
    renderer.Configure();
    renderer.CreateBuffers();
    Lua::Lib();
    Lua::Source("./resources/scripts/config.lua");
    Lua::Source("./resources/scripts/main.lua");
    Lua::L.set_function("get_cursor_uv", &Input::GetCursorUV, &input);
    Lua::L.set_function("get_key_down", &Input::GetKeyDown, &input);
    //Lua::L.set_function("check_errors", &Renderer::CheckErrors, renderer);
    Lua::Run("init()");
    this->_initialized = true;
}

void Runtime::Execute()
{
    Load();
    _mb->Supervise(this);
    cout << "[Engine] Game fully loaded." << endl;
    
    double lastFrameTime = this->_fw->GetTime();
    while (!this->_fw->Run() && !this->_quitted)
    {
        //this->_mb->Notify();

        float currentTime = this->_fw->GetTime();
        float currentFrameTime = currentTime;
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        currentTime = this->_fw->GetTime();
        this->Update(deltaTime, currentTime);
        float ut = this->_fw->GetTime();
        //cout << "Update cost: " << ut - currentTime << "secs" << endl;

        currentTime = this->_fw->GetTime();
        this->Render(deltaTime, currentTime);
        float rt = this->_fw->GetTime();
        //cout << "Render cost: " << rt - currentTime << "secs" << endl;

        this->_fw->Draw();
    }
    cout << "[Engine] Game quitted." << endl;
}

void Runtime::Quit()
{
    cout << "[Engine] Requested to quit." << endl;
    this->_quitted = true;
}

void Runtime::Load()
{

}

void Runtime::Update(float dt, float time)
{
    
}

void Runtime::Render(float dt, float time)
{
    
}

void Runtime::RenderGUI(float dt)
{
    
}
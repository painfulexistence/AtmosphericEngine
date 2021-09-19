#pragma once
#include "Globals.hpp"
#include "Framework.hpp"
#include "Physics.hpp"
#include "Graphics.hpp"
#include "GUI.hpp"
#include "Console.hpp"
#include "Input/Input.hpp"
#include "Scripting.hpp"

class Runtime
{
private:
    bool _initialized = false;
    bool _quitted = false;
    // The framework needs to be initialized first before the constructor call, so that the graphics library gets loaded earlier than other things do.
    // Do not put the framework on the stack, or try to initialize it elsewhere, otherwise segment fault may creep in unexpecedly.
    Application* _app = new Application();
    MessageBus* _mb = new MessageBus(this);
    Window* _win = nullptr;
        
    void Log(std::string message);

    float Time();

    void Process(float dt);

    void Render(float dt); // TODO: Separate rendering and drawing logic if the backend supports command buffering

public:
    Runtime();

    ~Runtime();
        
    void Execute();

    void Quit();

    virtual void Load() = 0;

    virtual void Update(float dt, float time) = 0;

protected:
    // These subsystems will be game accessible
    Renderer renderer;
    PhysicsWorld world;
    Console console;
    GUI gui;
    Input input;
    Scene scene;
    Script script;
    std::list<Entity>& entities;
    std::vector<Camera> _cameras = {};
    std::vector<Light> _lights = {};
    std::vector<Material> _materials = {};
    glm::vec4 clearColor = glm::vec4(0.15f, 0.183f, 0.2f, 1.0f);
    ShaderProgram colorProgram;
    ShaderProgram depthTextureProgram;
    ShaderProgram depthCubemapProgram;
    ShaderProgram hdrProgram;
};
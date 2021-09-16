#pragma once
#include "common.hpp"
#include "Framework.hpp"
#include "Messaging.hpp"
#include "Physics.hpp"
#include "Graphics.hpp"
#include "GUI.hpp"
#include "Console.hpp"
#include "Input/Input.hpp"
#include "Scripting.hpp"
#include "Framework/ImGui.hpp" //TODO: Remove this dependency
//#include <entt/entity/registry.hpp>
using namespace std;

class Runtime
{
private:
    // The framework needs to be constructed first so that the graphics library gets loaded earlier than other things do.
    // Do not put the framework on the stack, otherwise it may be deconstructed unexpecedly.
    bool _initialized = false;
    bool _quitted = false;
    Framework* _fw = new Framework();
    MessageBus* _mb = new MessageBus();
    void Render(float dt, float time);

public:
    Runtime();

    ~Runtime();
        
    void Execute();

    void Quit();

    virtual void Load() = 0;

    virtual void Update(float dt, float time) = 0;

protected:
    Renderer renderer;
    PhysicsWorld world;
    Console console;
    GUI gui;
    Input input;
    Scene scene;
    Script script;
    list<Entity>& entities;
    vector<Camera> _cameras = {};
    vector<Light> _lights = {};
    vector<Material> _materials = {};
    ShaderProgram colorProgram;
    ShaderProgram depthTextureProgram;
    ShaderProgram depthCubemapProgram;
    ShaderProgram hdrProgram;
    ImVec4 clearColor = ImVec4(0.15f, 0.183f, 0.2f, 1.0f);
};
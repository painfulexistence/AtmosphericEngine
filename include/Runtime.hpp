#pragma once
#include "common.hpp"
#include "Framework.hpp"
#include "Messaging.hpp"
#include "Physics.hpp"
#include "Graphics.hpp"
#include "Console.hpp"
#include "Input/Input.hpp"
#include "Scripting/lua.hpp"
//#include <entt/entity/registry.hpp>
using namespace std;

const ImVec4 clearColor = ImVec4(0.15f, 0.183f, 0.2f, 1.0f);

class Runtime
{
private:
    // The framework needs to be constructed first so that the graphics library gets loaded earlier than other things do.
    // Do not put the framework on the stack, otherwise it may be deconstructed unexpecedly.
    bool _initialized = false;
    bool _quitted = false;
    Framework* _fw = new Framework();
    MessageBus* _mb = new MessageBus();

public:
    Runtime();

    ~Runtime();
        
    void Init();

    void Execute();

    void Quit();

    virtual void Load();

    virtual void Update(float dt, float time);

    virtual void Render(float dt, float time);

    virtual void RenderGUI(float dt);

protected:
    Renderer renderer;
    PhysicsWorld world;
    Console console;
    Input input;
    Scene scene;
    Lua lua;
    list<Entity>& entities;
    vector<Camera> _cameras = {};
    vector<Light> _lights = {};
    vector<Material> _materials = {};
    ShaderProgram colorProgram;
    ShaderProgram depthTextureProgram;
    ShaderProgram depthCubemapProgram;
    ShaderProgram hdrProgram;
};
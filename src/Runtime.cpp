#include "Runtime.hpp"
#include <iostream> // Note that should only use IO when debugging
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

Runtime::Runtime() : entities(Entity::Entities)
{
    Log("Launching...");
    this->_win = this->_app->GetActiveWindow(); // Multi-window not supported now
}

Runtime::~Runtime()
{
    Log("Exiting...");
    delete this->_mb;
    delete this->_app;
}

void Runtime::Execute()
{
    Log("Initializing...");
    console.Init(_mb, _app);
    gui.Init(_mb, _app);
    input.Init(_mb, _app);
    world.Init(_mb, _app);
    renderer.Init(_mb, _app);
    script.Init(_mb, _app);
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
        #if SINGLE_THREAD
        #pragma region main_loop_st
        Process(deltaTime);
        for (const auto& ent : Entity::WithPhysicsComponent())
        {
            const glm::mat4& m2w = ConvertPhysicalMatrix(world.GetImpostorTransform(ent.GetPhysicsId()));
            scene.SetGeometryModelWorldTransform(ent.GetGraphicsId(), m2w);
        }
        Render(deltaTime);
        #pragma endregion
        #else
        #pragma region main_loop_mt
        // Gameplay
        std::thread simulation(&Runtime::Process, this, deltaTime);
        // Graphics
        Render(deltaTime);
        // Sync
        simulation.join();
        for (const auto& ent : Entity::WithPhysicsComponent())
        {
            const glm::mat4& m2w = ConvertPhysicalMatrix(world.GetImpostorTransform(ent.GetPhysicsId()));
            scene.SetGeometryModelWorldTransform(ent.GetGraphicsId(), m2w);
        }
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

void Runtime::Process(float dt)
{
    float time = Time();
    
    Update(dt, time);
    this->_mb->Process();

    console.Process(dt);
    input.Process(dt);
    script.Process(dt);
    world.Process(dt);
    world.DampenImpostor(_cameras[0].GetPhysicsId());

    #if SHOW_PROCESS_COST
    Log(fmt::format("Update costs {} ms", (Time() - time) * 1000));
    #endif
}

void Runtime::Render(float dt)
{
    float time = Time();

    const int mainLightCount = 1;
    const int auxLightCount = (int)_lights.size() - mainLightCount;

    renderer.BindSceneVAO();
    
    renderer.BeginShadowPass();
    {
        int auxShadows = 0;
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderer.GetShadowMap(DIR_LIGHT, 0), 0);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthTextureProgram.Activate();
        scene.Render(depthTextureProgram, _lights[0].GetProjectionMatrix(0), _lights[0].GetViewMatrix());
        for (int i = 0; i < auxLightCount; ++i)
        {
            Light& l = _lights[i + mainLightCount];
            if ((bool)l.castShadow)
                continue;
            if (auxShadows++ >= MAX_AUX_SHADOWS)
                break;

            for (int f = 0; f < 6; ++f)
            {
                GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + f;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, renderer.GetShadowMap(POINT_LIGHT, i), 0);
                glClear(GL_DEPTH_BUFFER_BIT);
                depthCubemapProgram.Activate();
                depthCubemapProgram.SetUniform(string("LightPosition"), l.position);
                scene.Render(depthCubemapProgram, l.GetProjectionMatrix(0), l.GetViewMatrix(face));
            }
        }
    }
    renderer.EndShadowPass();
    
    renderer.BeginColorPass();
    {
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);    
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        colorProgram.Activate();
        glm::mat4 cameraTransform = scene.GetGeometryWorldMatrix(_cameras[0].GetGraphicsId());
        glm::vec3 eyePos = _cameras[0].GetEye(cameraTransform);
        colorProgram.SetUniform(string("cam_pos"), eyePos);
        colorProgram.SetUniform(string("time"), Time());
        colorProgram.SetUniform(string("main_light.direction"), _lights[0].direction);
        colorProgram.SetUniform(string("main_light.ambient"), _lights[0].ambient);
        colorProgram.SetUniform(string("main_light.diffuse"), _lights[0].diffuse);
        colorProgram.SetUniform(string("main_light.specular"), _lights[0].specular);
        colorProgram.SetUniform(string("main_light.intensity"), _lights[0].intensity);
        colorProgram.SetUniform(string("main_light.cast_shadow"), _lights[0].castShadow);
        colorProgram.SetUniform(string("main_light.ProjectionView"), _lights[0].GetProjectionViewMatrix(0));
        for (int i = 0; i < auxLightCount; ++i)
        {
            Light& l = _lights[i + mainLightCount];
            colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].position"), l.position);
            colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].ambient"), l.ambient);
            colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].diffuse"), l.diffuse);
            colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].specular"), l.specular);
            colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].attenuation"), l.attenuation);
            colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].intensity"), l.intensity);
            colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].cast_shadow"), l.castShadow);
            for (int f = 0; f < 6; ++f)
            {
                GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + f;
                colorProgram.SetUniform(string("aux_lights[") + to_string(i) + string("].ProjectionViews[") + to_string(f) + string("]"), l.GetProjectionViewMatrix(0, face));
            }
        }
        colorProgram.SetUniform(string("aux_light_count"), auxLightCount);
        colorProgram.SetUniform(string("shadow_map_unit"), (int)0);
        colorProgram.SetUniform(string("omni_shadow_map_unit"), (int)1);
        scene.Render(colorProgram, _cameras[0].GetProjectionMatrix(), _cameras[0].GetViewMatrix(cameraTransform));
    }
    renderer.EndColorPass();

    renderer.BindScreenVAO();

    renderer.BeginScreenColorPass();
    {
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);    
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        hdrProgram.Activate();
        hdrProgram.SetUniform(string("color_map_unit"), (int)0);
        //hdrProgram.SetUniform(string("exposure"), (float)1.0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    renderer.EndScreenColorPass();
    
    gui.Render(dt);

    this->_win->SwapBuffers();

    #if SHOW_RENDER_AND_DRAW_COST
    Log(fmt::format("Render & draw cost {} ms", (Time() - time) * 1000));
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
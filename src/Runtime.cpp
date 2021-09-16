#include "Runtime.hpp"
using namespace std;

Runtime::Runtime() : entities(Entity::Entities)
{
    cout << "Launching..." << endl;
}

Runtime::~Runtime()
{
    cout << "Exiting..." << endl;
    delete _mb;
    delete _fw;
}

void Runtime::Execute()
{
    cout << "Initializing..." << endl;
    //setbuf(stdout, NULL); //Cancel output stream buffering so that output can be seen immediately
    srand(time(NULL)); //Dont's use glfwGetTime() bc it only starts to calculate time since window was created
    console.Init(_mb, _fw);
    gui.Init(_mb, _fw);
    input.Init(_mb, _fw);
    world.Init(_mb, _fw);
    //renderer.Init(_mb, _fw);
    renderer.Configure();
    renderer.CreateBuffers();
    script.Init(_mb, _fw);
    this->_initialized = true;
    
    Load();
    _mb->Supervise(this);
    cout << "Game fully loaded." << endl;
    
    double lastFrameTime = this->_fw->GetTime();
    while (!this->_fw->Run() && !this->_quitted)
    {
        //this->_mb->Notify();

        float currentTime = this->_fw->GetTime();
        float currentFrameTime = currentTime;
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        currentTime = this->_fw->GetTime();
        Update(deltaTime, currentTime);
        float ut = this->_fw->GetTime();
        //cout << "Update cost: " << ut - currentTime << "secs" << endl;

        currentTime = this->_fw->GetTime();
        Render(deltaTime, currentTime);
        float rt = this->_fw->GetTime();
        //cout << "Render cost: " << rt - currentTime << "secs" << endl;

        this->_fw->Draw();
    }
    cout << "Game quitted." << endl;
}

void Runtime::Quit()
{
    cout << "Requested to quit." << endl;
    this->_quitted = true;
}

void Runtime::Render(float dt, float time)
{
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
            if (l.castShadow == 0)
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
        colorProgram.SetUniform(string("time"), time);
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
    
    gui.Render();

    Lua::L["dt"] = dt;
    Lua::Run("draw(dt)");   
}
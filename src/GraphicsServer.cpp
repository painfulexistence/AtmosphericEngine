#include "GraphicsServer.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "GameObject.hpp"

GraphicsServer::GraphicsServer()
{

}

GraphicsServer::~GraphicsServer()
{
    for (auto& mat : materials)
        delete mat;
    glDeleteTextures(textures.size(), textures.data());
}

void GraphicsServer::Init(MessageBus* mb, Application* app)
{
    Server::Init(mb, app);

    // Note that OpenGL extensions must NOT be initialzed before the window creation
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize glew!");
    
    glPrimitiveRestartIndex(0xFFFF);
    glCullFace(GL_BACK);
    #if MSAA_ON
    glEnable(GL_MULTISAMPLE);
    #endif

    this->_app->GetActiveWindow()->SetOnFramebufferResize([this](int width, int height) {
        if (this->_app->GetActiveWindow()->IsClosing())
            return;
        this->_fbProps.width = width;
        this->_fbProps.height = height;
        this->ResetFramebuffers(); // TODO: Reuse framebuffers -- just resize them
    });

    this->ResetFramebuffers();
    this->ResetScreenVAO();
}

void GraphicsServer::Process(float dt)
{

}

void GraphicsServer::Render(float dt)
{
    // TODO: Put the logic of generating command buffers here
    // Setup
    modelInstancesMap.clear();
    for (auto mesh : meshes)
    {
        Model* model = mesh->model;
        glm::mat4 wm = mesh->gameObject->GetTransform();
        if (modelInstancesMap.count(model) == 0)
            modelInstancesMap.insert({model, std::vector<glm::mat4>(0)});
        modelInstancesMap.find(model)->second.push_back(wm);
    }
    auxLightCount = (int)lights.size() - mainLightCount;
    
    //ShadowPass(dt);

    ColorPass(dt);
    
    PostProcessPass(dt);
}

void GraphicsServer::OnMessage(Message msg)
{

}

void GraphicsServer::LoadTexture(const std::string& path)
{ 
    GLuint tex;
    glGenTextures(1, &tex);
    textures.push_back(tex);    
    
    glBindTexture(GL_TEXTURE_2D, tex);
    float border[] = {1.f, 1.f, 1.f, 1.f};
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);  
    
    int width, height, nChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        throw std::runtime_error(fmt::format("Failed to load texture at {}\n", path));
    }
    stbi_image_free(data);
}

void GraphicsServer::LoadTextures(const std::vector<std::string>& paths)
{
    int count = paths.size();
    textures.resize(count);
    glGenTextures(count, textures.data());
    
    for (int i = 0; i < count; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        float border[] = {1.f, 1.f, 1.f, 1.f};
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);  
        
        int width, height, nChannels;
        unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            throw std::runtime_error(fmt::format("Failed to load texture at {}\n", paths[i]));
        }
        stbi_image_free(data);
    }
}

void GraphicsServer::CheckErrors()
{     
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode)
        {
            // Reference: https://learnopengl.com/In-Practice/Debugging
            case GL_INVALID_ENUM:                  
            error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 
            error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             
            error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                
            error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               
            error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 
            error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: 
            error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        throw std::runtime_error(fmt::format("GL error: {}\n", error));
    }
}

void GraphicsServer::ResetFramebuffers()
{
    // Allocate framebuffers
    glGenFramebuffers(1, &shadowFBO);
    for (int i = 0; i < 1; ++i)
    {
        GLuint map;
        glGenTextures(1, &map);
        glBindTexture(GL_TEXTURE_2D, map);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        uniShadowMaps.push_back(map);
    }
    for (int i = 0; i < MAX_AUX_SHADOWS; ++i)
    {
        GLuint map;
        glGenTextures(1, &map);
        glBindTexture(GL_TEXTURE_CUBE_MAP, map);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        for (int f = 0; f < 6; ++f)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); 
        }
        omniShadowMaps.push_back(map);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    for (int i = 0; i < (int)uniShadowMaps.size(); ++i)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, uniShadowMaps[0], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    for (int i = 0; i < (int)omniShadowMaps.size(); ++i)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, omniShadowMaps[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &hdrFBO);
    glGenTextures(1, &hdrColorTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, hdrColorTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, this->_fbProps.numSapmples, GL_RGBA16F, this->_fbProps.width, this->_fbProps.height, GL_TRUE); 
    glGenTextures(1, &hdrDepthTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, hdrDepthTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, this->_fbProps.numSapmples, GL_DEPTH_COMPONENT, this->_fbProps.width, this->_fbProps.height, GL_TRUE); 
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, hdrColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, hdrDepthTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::runtime_error("HDR Framebuffer is incomplete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &msaaFBO);
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, this->_fbProps.width, this->_fbProps.height, 0, GL_RGBA, GL_FLOAT, NULL); 
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::runtime_error("MSAA Framebuffer is incomplete!");
}

void GraphicsServer::ResetScreenVAO()
{
    std::vector<GLfloat> verts = {
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);

    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void GraphicsServer::ShadowPass(float dt)
{
    glViewport(0, 0, SHADOW_W, SHADOW_H);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GetShadowMap(DIR_LIGHT, 0), 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    depthTextureProgram.Activate();
    depthTextureProgram.SetUniform(std::string("ProjectionView"), lights[0]->GetProjectionMatrix(0) * lights[0]->GetViewMatrix());

    for (const auto& [name, model] : Model::ModelList)
    {
        if (modelInstancesMap.count(model) == 0)
            continue;
        model->Render(depthTextureProgram, modelInstancesMap.find(model)->second);
    }

    int auxShadows = 0;
    for (int i = 0; i < auxLightCount; ++i)
    {
        Light* l = lights[i + mainLightCount];
        if ((bool)l->castShadow)
            continue;
        if (auxShadows++ >= MAX_AUX_SHADOWS)
            break;

        for (int f = 0; f < 6; ++f)
        {
            GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + f;
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, GetShadowMap(POINT_LIGHT, i), 0);
            glClear(GL_DEPTH_BUFFER_BIT);
            depthCubemapProgram.Activate();
            depthCubemapProgram.SetUniform(std::string("LightPosition"), l->position);
            depthCubemapProgram.SetUniform(std::string("ProjectionView"), l->GetProjectionMatrix(0) * l->GetViewMatrix(face));

            for (const auto& [name, model] : Model::ModelList)
            {
                if (modelInstancesMap.count(model) == 0)
                    continue;
                model->Render(depthCubemapProgram, modelInstancesMap.find(model)->second);
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsServer::ColorPass(float dt)
{
    glViewport(0, 0, this->_fbProps.width, this->_fbProps.height);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, uniShadowMaps[0]);
    for (int i = 0; i < MAX_AUX_SHADOWS; ++i)
    {
        glActiveTexture(GL_TEXTURE1 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, omniShadowMaps[i]);
    }
    for (int i = 0; i < textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + NUM_MAP_UNITS + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    colorProgram.Activate();
    glm::mat4 cameraTransform = cameras[0]->gameObject->GetModelWorldTransform();
    glm::vec3 eyePos = cameras[0]->GetEye(cameraTransform);
    colorProgram.SetUniform(std::string("cam_pos"), eyePos);
    colorProgram.SetUniform(std::string("time"), 0);
    colorProgram.SetUniform(std::string("main_light.direction"), lights[0]->direction);
    colorProgram.SetUniform(std::string("main_light.ambient"), lights[0]->ambient);
    colorProgram.SetUniform(std::string("main_light.diffuse"), lights[0]->diffuse);
    colorProgram.SetUniform(std::string("main_light.specular"), lights[0]->specular);
    colorProgram.SetUniform(std::string("main_light.intensity"), lights[0]->intensity);
    colorProgram.SetUniform(std::string("main_light.cast_shadow"), lights[0]->castShadow);
    colorProgram.SetUniform(std::string("main_light.ProjectionView"), lights[0]->GetProjectionViewMatrix(0));
    for (int i = 0; i < auxLightCount; ++i)
    {
        Light* l = lights[i + mainLightCount];
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].position"), l->position);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].ambient"), l->ambient);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].diffuse"), l->diffuse);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].specular"), l->specular);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].attenuation"), l->attenuation);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].intensity"), l->intensity);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].cast_shadow"), l->castShadow);
        for (int f = 0; f < 6; ++f)
        {
            GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + f;
            colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].ProjectionViews[") + std::to_string(f) + std::string("]"), l->GetProjectionViewMatrix(0, face));
        }
    }
    colorProgram.SetUniform(std::string("aux_light_count"), auxLightCount);
    colorProgram.SetUniform(std::string("shadow_map_unit"), (int)0);
    colorProgram.SetUniform(std::string("omni_shadow_map_unit"), (int)1); 
    colorProgram.SetUniform(std::string("ProjectionView"), cameras[0]->GetProjectionMatrix() * cameras[0]->GetViewMatrix(cameraTransform));

    for (const auto& [name, model] : Model::ModelList)
    {
        if (modelInstancesMap.count(model) == 0)
            continue;
        model->Render(colorProgram, modelInstancesMap.find(model)->second);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsServer::PostProcessPass(float dt)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
    glBlitFramebuffer(0, 0, this->_fbProps.width, this->_fbProps.height, 0, 0, this->_fbProps.width, this->_fbProps.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
    glViewport(0, 0, this->_fbProps.width, this->_fbProps.height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    hdrProgram.Activate();
    hdrProgram.SetUniform(std::string("color_map_unit"), (int)0);
    //hdrProgram.SetUniform(std::string("exposure"), (float)1.0);
    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
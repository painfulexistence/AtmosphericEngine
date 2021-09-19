#pragma once
#include "Globals.hpp"
#include "Messaging.hpp"
#include "Framework.hpp"

class Renderer : Server
{
    GLuint shadowFBO, hdrFBO, msaaFBO;
    std::vector<GLuint> uniShadowMaps;
    std::vector<GLuint> omniShadowMaps;
    GLuint hdrColorTexture, hdrDepthTexture;
    GLuint screenTexture;
    std::vector<GLuint> textures;
    GLuint sceneVAO, screenVAO;
    GLuint screenVBO;

public:
    Renderer();

    void Init(MessageBus* mb, Application* app);

    void OnMessage(Message msg) override;

    void CreateTexture(const std::string&);
    
    void BindSceneVAO();
    
    void BindScreenVAO();
    
    void BeginShadowPass();
    
    void EndShadowPass();
    
    void BeginColorPass();
    
    void EndColorPass();
    
    void BeginScreenColorPass();
    
    void EndScreenColorPass();

    void CheckErrors();

    GLuint GetShadowMap(int lightType, int lightIdx = 0)
    {
        if (lightType == DIR_LIGHT)
        {
            return uniShadowMaps[lightIdx];
        }
        return omniShadowMaps[lightIdx];
    }

    void SetCapability(const GLenum& cap, bool enable = true)
    {
        if (enable) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }
    }

};
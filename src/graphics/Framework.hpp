#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../common.hpp"

class Framework
{
    GLFWwindow* window = nullptr;
    GLuint shadowFBO, hdrFBO, msaaFBO;
    std::vector<GLuint> uniShadowMaps;
    std::vector<GLuint> omniShadowMaps;
    GLuint hdrColorTexture, hdrDepthTexture;
    GLuint screenTexture;
    std::vector<GLuint> textures;
    GLuint sceneVAO, screenVAO;
    GLuint screenVBO;

public:
    void Init();
    void CreateTexture(const std::string&);
    void BindSceneVAO();
    void BindScreenVAO();
    void BeginShadowPass();
    void EndShadowPass();
    void BeginColorPass();
    void EndColorPass();
    void BeginScreenColorPass();
    void EndScreenColorPass();
    void SwapBuffers();
    void PollEvents();
    void CloseWindow();
    void Terminate();
    void CheckErrors();

    GLuint GetShadowMap(int lightType, int lightIdx = 0)
    {
        if (lightType == DIR_LIGHT)
        {
            return uniShadowMaps[lightIdx];
        }
        return omniShadowMaps[lightIdx];
    }

    double GetTime() { return glfwGetTime(); }
    
    glm::vec2 GetCursorPosition() 
    {
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        return glm::vec2(xPos, yPos);
    };
    
    glm::vec2 GetCursorUV()
    {
        glm::vec2 cursorPos = GetCursorPosition();
        return glm::vec2(cursorPos.x / SCREEN_W, cursorPos.y / SCREEN_H);
    };

    bool IsWindowOpen()
    {
        return glfwWindowShouldClose(window);
    };

    bool IsKeyDown(int key)
    {
        return (glfwGetKey(window, key) == GLFW_PRESS);
    };

    void SetRendererCapability(const GLenum& cap, bool enable = true)
    {
        if (enable) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }
    };
};

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../common.hpp"

class Framework
{
    GLFWwindow* window = nullptr;
    GLuint shadowFramebuffer;
    GLuint hdrFramebuffer;
    GLuint screenVAO;
    GLuint screenVBO;
    
    std::vector<GLuint> textures;
    std::vector<GLuint> uniShadowMaps;
    std::vector<GLuint> omniShadowMaps;
    GLuint hdrColorMap;
    GLuint hdrDepthMap;
    GLuint normalMap;
    GLuint diffuseMap;
    GLuint specularMap;
    GLuint metallicMap;

public:
    void Init();
    void CreateTexture(const std::string&);
    void Blit();
    void SwapBuffers();
    void PollEvents();
    void CloseWindow();
    void Terminate();
    void CheckErrors();

    std::vector<GLuint> GetTextures() { return textures; }

    GLuint GetHDRFramebuffer() { return hdrFramebuffer; }

    GLuint GetHDRColorMap() { return hdrColorMap; }

    GLuint GetShadowFramebuffer() { return shadowFramebuffer; }

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

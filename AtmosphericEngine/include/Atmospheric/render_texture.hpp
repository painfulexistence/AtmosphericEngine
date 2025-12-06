#pragma once

#include "globals.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

// RenderTexture - Offscreen render target for 2D and 3D rendering
// Can be used as:
// - Canvas for 2D drawing (minimap, UI caching)
// - Render target for 3D (reflections, portals)
// - Post-processing intermediate buffer
class RenderTexture {
public:
    struct Props {
        int width = 256;
        int height = 256;
        bool withDepth = false;
        bool withStencil = false;
        bool hdr = false;  // Use RGBA16F instead of RGBA8
        bool filtered = true;  // Linear vs Nearest filtering
    };

    RenderTexture(int width, int height, bool withDepth = false);
    RenderTexture(const Props& props);
    ~RenderTexture();

    // Non-copyable
    RenderTexture(const RenderTexture&) = delete;
    RenderTexture& operator=(const RenderTexture&) = delete;

    // Movable
    RenderTexture(RenderTexture&& other) noexcept;
    RenderTexture& operator=(RenderTexture&& other) noexcept;

    // Begin rendering to this texture
    // Saves the previous framebuffer binding
    void Begin();

    // End rendering to this texture
    // Restores the previous framebuffer binding
    void End();

    // Clear the render texture
    void Clear(const glm::vec4& color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

    // Get the color texture ID (can be used as a regular texture)
    uint32_t GetTextureID() const { return _colorTexture; }

    // Get the depth texture ID (if created with depth)
    uint32_t GetDepthTextureID() const { return _depthTexture; }

    // Get dimensions
    int GetWidth() const { return _width; }
    int GetHeight() const { return _height; }
    glm::vec2 GetSize() const { return glm::vec2(_width, _height); }

    // Get projection matrix for 2D rendering (origin at top-left)
    glm::mat4 GetProjectionMatrix() const {
        return glm::ortho(0.0f, (float)_width, (float)_height, 0.0f, -1.0f, 1.0f);
    }

    // Get projection matrix for 2D rendering (origin at bottom-left, OpenGL style)
    glm::mat4 GetProjectionMatrixFlipped() const {
        return glm::ortho(0.0f, (float)_width, 0.0f, (float)_height, -1.0f, 1.0f);
    }

    // Check if valid
    bool IsValid() const { return _fbo != 0 && _colorTexture != 0; }

    // Resize the render texture (recreates internal resources)
    void Resize(int width, int height);

private:
    void Create();
    void Destroy();

    GLuint _fbo = 0;
    GLuint _colorTexture = 0;
    GLuint _depthTexture = 0;
    GLuint _depthStencilRBO = 0;

    int _width = 0;
    int _height = 0;
    bool _withDepth = false;
    bool _withStencil = false;
    bool _hdr = false;
    bool _filtered = true;

    // For nested Begin/End support
    GLint _prevFBO = 0;
    GLint _prevViewport[4] = {0, 0, 0, 0};
};

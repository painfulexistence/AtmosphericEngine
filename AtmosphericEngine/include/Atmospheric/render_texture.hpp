#pragma once
#include "globals.hpp"
#include "gpu_render_target.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// OpenGL implementation of IGPURenderTarget.
// Wraps an FBO with a color texture attachment and optional depth/stencil attachment.
class RenderTexture : public IGPURenderTarget {
public:
    // Backward-compatible alias — existing RenderTexture::Props{...} syntax still works.
    using Props = IGPURenderTarget::Props;

    RenderTexture(int width, int height, bool withDepth = false);
    RenderTexture(const Props& props);
    ~RenderTexture() override;

    RenderTexture(const RenderTexture&) = delete;
    RenderTexture& operator=(const RenderTexture&) = delete;
    RenderTexture(RenderTexture&& other) noexcept;
    RenderTexture& operator=(RenderTexture&& other) noexcept;

    // IGPURenderTarget interface — ctx is unused for GL.
    void Begin(IGPUCommandContext* ctx = nullptr) override;
    void End() override;
    void Clear(const glm::vec4& color = glm::vec4(0.0f)) override;

    uint32_t GetTextureID() const override { return _colorTexture; }
    uint32_t GetDepthTextureID() const override { return _depthTexture; }

    int GetWidth() const override { return _width; }
    int GetHeight() const override { return _height; }
    glm::vec2 GetSize() const override { return glm::vec2(_width, _height); }

    bool IsValid() const override { return _fbo != 0 && _colorTexture != 0; }

    void Resize(int width, int height) override;

    // 2D projection helpers
    glm::mat4 GetProjectionMatrix() const {
        return glm::ortho(0.0f, (float)_width, (float)_height, 0.0f, -1.0f, 1.0f);
    }
    glm::mat4 GetProjectionMatrixFlipped() const {
        return glm::ortho(0.0f, (float)_width, 0.0f, (float)_height, -1.0f, 1.0f);
    }

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

    GLint _prevFBO = 0;
    GLint _prevViewport[4] = {0, 0, 0, 0};
};

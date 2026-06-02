#pragma once
#include "gpu_command_context.hpp"
#include <cstdint>
#include <glm/glm.hpp>

// Abstract offscreen render target (framebuffer + color/depth attachments).
//
// GL backend:   pass nullptr (or omit) for ctx in Begin() — FBO state is implicit.
// SDL3 GPU:     pass an SDLGPUCommandContext with cmdBuf set; Begin() will create
//               the render pass and populate ctx->renderPass for subsequent draws.
class IGPURenderTarget {
public:
    struct Props {
        int width = 256;
        int height = 256;
        bool withDepth = false;
        bool withStencil = false;
        bool hdr = false;      // RGBA16F instead of RGBA8
        bool filtered = true;  // Linear vs Nearest sampling
    };

    virtual ~IGPURenderTarget() = default;

    // Bind this target. GL: saves + restores previous FBO. SDL3 GPU: begins render pass.
    virtual void Begin(IGPUCommandContext* ctx = nullptr) = 0;
    // Unbind / end render pass and restore previous state.
    virtual void End() = 0;
    // Set clear colour for the next Begin(). For GL this clears immediately if already bound.
    virtual void Clear(const glm::vec4& color = glm::vec4(0.0f)) = 0;

    // Color attachment handle (GLuint / low bits of SDL_GPUTexture*).
    // Use the concrete type's GetNativeTexture() for the typed handle on SDL3 GPU.
    virtual uint32_t GetTextureID() const = 0;
    // Depth attachment handle, or 0 if not created with a depth attachment.
    virtual uint32_t GetDepthTextureID() const = 0;

    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual glm::vec2 GetSize() const = 0;

    virtual bool IsValid() const = 0;
    virtual void Resize(int width, int height) = 0;
};

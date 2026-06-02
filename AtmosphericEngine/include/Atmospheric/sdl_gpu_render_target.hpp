#pragma once
#ifndef __EMSCRIPTEN__
#include "gpu_render_target.hpp"
#include "gpu_command_context.hpp"
#include <SDL3/SDL_gpu.h>
#include <glm/glm.hpp>

// SDL3 GPU implementation of IGPURenderTarget.
//
// Begin(ctx): starts a SDL_GPURenderPass targeting _colorTexture (and optionally
//             _depthTexture). Sets ctx->renderPass so subsequent Draw() calls can
//             bind their buffers to the active pass.
// End():      ends the render pass.
// Clear():    stores the clear colour; applied via load_op=CLEAR on the next Begin().
//
// GetTextureID() / GetDepthTextureID() return the low 32 bits of the SDL_GPUTexture*
// pointer for interface compatibility. Use GetNativeTexture() for the typed handle
// when sampling in a shader via SDL_GPUSampler.
class SDLGPURenderTarget : public IGPURenderTarget {
public:
    SDLGPURenderTarget(SDL_GPUDevice* device, const IGPURenderTarget::Props& props);
    ~SDLGPURenderTarget() override;

    SDLGPURenderTarget(const SDLGPURenderTarget&) = delete;
    SDLGPURenderTarget& operator=(const SDLGPURenderTarget&) = delete;

    void Begin(IGPUCommandContext* ctx = nullptr) override;
    void End() override;
    void Clear(const glm::vec4& color = glm::vec4(0.0f)) override;

    uint32_t GetTextureID() const override;
    uint32_t GetDepthTextureID() const override;

    int GetWidth() const override { return _width; }
    int GetHeight() const override { return _height; }
    glm::vec2 GetSize() const override { return { (float)_width, (float)_height }; }

    bool IsValid() const override { return _colorTexture != nullptr; }
    void Resize(int width, int height) override;

    SDL_GPUTexture* GetNativeTexture() const { return _colorTexture; }
    SDL_GPUTexture* GetNativeDepthTexture() const { return _depthTexture; }

private:
    void Create();
    void Destroy();

    SDL_GPUDevice*     _device       = nullptr;
    SDL_GPUTexture*    _colorTexture = nullptr;
    SDL_GPUTexture*    _depthTexture = nullptr;
    SDL_GPURenderPass* _activePass   = nullptr;

    int _width     = 0;
    int _height    = 0;
    bool _withDepth = false;
    bool _hdr       = false;
    glm::vec4 _clearColor = glm::vec4(0.0f);
};
#endif // !__EMSCRIPTEN__

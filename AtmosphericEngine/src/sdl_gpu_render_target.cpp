#ifndef __EMSCRIPTEN__
#include "sdl_gpu_render_target.hpp"

SDLGPURenderTarget::SDLGPURenderTarget(SDL_GPUDevice* device,
                                       const IGPURenderTarget::Props& props)
    : _device(device),
      _width(props.width),
      _height(props.height),
      _withDepth(props.withDepth),
      _hdr(props.hdr) {
    Create();
}

SDLGPURenderTarget::~SDLGPURenderTarget() {
    Destroy();
}

void SDLGPURenderTarget::Create() {
    SDL_GPUTextureCreateInfo colorInfo{};
    colorInfo.type   = SDL_GPU_TEXTURETYPE_2D;
    colorInfo.format = _hdr ? SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT
                             : SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    colorInfo.usage  = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET |
                       SDL_GPU_TEXTUREUSAGE_SAMPLER;
    colorInfo.width              = static_cast<Uint32>(_width);
    colorInfo.height             = static_cast<Uint32>(_height);
    colorInfo.layer_count_or_depth = 1;
    colorInfo.num_levels         = 1;
    colorInfo.sample_count       = SDL_GPU_SAMPLECOUNT_1;
    _colorTexture = SDL_CreateGPUTexture(_device, &colorInfo);

    if (_withDepth) {
        SDL_GPUTextureCreateInfo depthInfo{};
        depthInfo.type   = SDL_GPU_TEXTURETYPE_2D;
        depthInfo.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
        depthInfo.usage  = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET |
                           SDL_GPU_TEXTUREUSAGE_SAMPLER;
        depthInfo.width              = static_cast<Uint32>(_width);
        depthInfo.height             = static_cast<Uint32>(_height);
        depthInfo.layer_count_or_depth = 1;
        depthInfo.num_levels         = 1;
        depthInfo.sample_count       = SDL_GPU_SAMPLECOUNT_1;
        _depthTexture = SDL_CreateGPUTexture(_device, &depthInfo);
    }
}

void SDLGPURenderTarget::Destroy() {
    if (_activePass) {
        SDL_EndGPURenderPass(_activePass);
        _activePass = nullptr;
    }
    if (_colorTexture) { SDL_ReleaseGPUTexture(_device, _colorTexture); _colorTexture = nullptr; }
    if (_depthTexture) { SDL_ReleaseGPUTexture(_device, _depthTexture); _depthTexture = nullptr; }
}

void SDLGPURenderTarget::Begin(IGPUCommandContext* ctx) {
    auto* sdlCtx = static_cast<SDLGPUCommandContext*>(ctx);

    SDL_GPUColorTargetInfo colorTarget{};
    colorTarget.texture    = _colorTexture;
    colorTarget.load_op    = SDL_GPU_LOADOP_CLEAR;
    colorTarget.store_op   = SDL_GPU_STOREOP_STORE;
    colorTarget.clear_color = { _clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a };

    bool hasDepth = _withDepth && _depthTexture;
    SDL_GPUDepthStencilTargetInfo depthTarget{};
    if (hasDepth) {
        depthTarget.texture     = _depthTexture;
        depthTarget.load_op     = SDL_GPU_LOADOP_CLEAR;
        depthTarget.store_op    = SDL_GPU_STOREOP_STORE;
        depthTarget.clear_depth = 1.0f;
    }

    _activePass = SDL_BeginGPURenderPass(
        sdlCtx->cmdBuf,
        &colorTarget, 1,
        hasDepth ? &depthTarget : nullptr
    );
    sdlCtx->renderPass = _activePass;
}

void SDLGPURenderTarget::End() {
    if (_activePass) {
        SDL_EndGPURenderPass(_activePass);
        _activePass = nullptr;
    }
}

void SDLGPURenderTarget::Clear(const glm::vec4& color) {
    // Stored and applied as load_op=CLEAR at the next Begin().
    _clearColor = color;
}

uint32_t SDLGPURenderTarget::GetTextureID() const {
    // Low 32 bits of the SDL_GPUTexture* pointer — sufficient for interface
    // compatibility. Use GetNativeTexture() for shader binding.
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(_colorTexture));
}

uint32_t SDLGPURenderTarget::GetDepthTextureID() const {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(_depthTexture));
}

void SDLGPURenderTarget::Resize(int width, int height) {
    if (width == _width && height == _height) return;
    _width  = width;
    _height = height;
    Destroy();
    Create();
}
#endif // !__EMSCRIPTEN__

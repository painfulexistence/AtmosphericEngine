#include "gfx_factory.hpp"

#ifdef __EMSCRIPTEN__
#if defined(AE_WEB_BACKEND_WEBGPU)
#include <webgpu/webgpu.h>
#include "webgpu_buffer.hpp"
#include "webgpu_render_target.hpp"
#else
// Legacy WebGL2 path — still uses the GL object wrappers.
#include "render_mesh.hpp"
#include "render_texture.hpp"
#endif
#else // native
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL.h>
#include "sdl_gpu_buffer.hpp"
#include "sdl_gpu_render_target.hpp"
#endif

// ── Static member definitions ────────────────────────────────────────────────
GfxBackend GfxFactory::_backend = GfxBackend::OpenGL; // overwritten in Init()

#ifdef __EMSCRIPTEN__
WGPUDevice GfxFactory::_wgpuDevice = nullptr;
WGPUQueue  GfxFactory::_wgpuQueue  = nullptr;
#else
SDL_GPUDevice* GfxFactory::_sdlDevice = nullptr;
#endif

// ── Init ─────────────────────────────────────────────────────────────────────
#ifdef __EMSCRIPTEN__

void GfxFactory::Init() {
    _backend = GfxBackend::WebGPU;
    // Device is requested asynchronously via navigator.gpu.requestAdapter().
    // The caller must invoke SetWebGPUDevice() once the callback fires.
}

void GfxFactory::SetWebGPUDevice(WGPUDevice device) {
#if defined(AE_WEB_BACKEND_WEBGPU)
    _wgpuDevice = device;
    _wgpuQueue  = wgpuDeviceGetQueue(device);
#endif
}

#else // native

void GfxFactory::Init(SDL_Window* sdlWindow) {
    _backend = GfxBackend::SDLGPU;

    /* TODO: Priority / availability detection.
       Uncomment to fall back to the legacy OpenGL path if SDL3 GPU is
       unavailable on this system (e.g. older Vulkan driver).

    SDL_GPUShaderFormat needed =
        SDL_GPU_SHADERFORMAT_SPIRV  |
        SDL_GPU_SHADERFORMAT_DXIL   |
        SDL_GPU_SHADERFORMAT_MSL;
    if (!SDL_GPUSupportsShaderFormats(needed, nullptr)) {
        _backend = GfxBackend::OpenGL;
        return;
    }
    */

    const SDL_GPUShaderFormat formats =
        SDL_GPU_SHADERFORMAT_SPIRV    |
        SDL_GPU_SHADERFORMAT_DXBC     |
        SDL_GPU_SHADERFORMAT_DXIL     |
        SDL_GPU_SHADERFORMAT_MSL      |
        SDL_GPU_SHADERFORMAT_METALLIB;

    _sdlDevice = SDL_CreateGPUDevice(formats, /*debug=*/false, /*name=*/nullptr);
    if (sdlWindow && _sdlDevice) {
        SDL_ClaimWindowForGPUDevice(_sdlDevice, sdlWindow);
    }
}

#endif

// ── Shutdown ─────────────────────────────────────────────────────────────────
void GfxFactory::Shutdown() {
#ifdef __EMSCRIPTEN__
#if defined(AE_WEB_BACKEND_WEBGPU)
    if (_wgpuDevice) {
        wgpuDeviceRelease(_wgpuDevice);
        _wgpuDevice = nullptr;
        _wgpuQueue  = nullptr;
    }
#endif
#else
    if (_sdlDevice) {
        SDL_DestroyGPUDevice(_sdlDevice);
        _sdlDevice = nullptr;
    }
#endif
}

// ── Factory methods ──────────────────────────────────────────────────────────
std::unique_ptr<IGPUBuffer> GfxFactory::CreateBuffer() {
#ifdef __EMSCRIPTEN__
#if defined(AE_WEB_BACKEND_WEBGPU)
    return std::make_unique<WebGPUBuffer>(_wgpuDevice, _wgpuQueue);
#else
    return std::make_unique<RenderMesh>(); // legacy WebGL2
#endif
#else
    return std::make_unique<SDLGPUBuffer>(_sdlDevice);
#endif
}

std::unique_ptr<IGPURenderTarget> GfxFactory::CreateRenderTarget(
        const IGPURenderTarget::Props& props) {
#ifdef __EMSCRIPTEN__
#if defined(AE_WEB_BACKEND_WEBGPU)
    return std::make_unique<WebGPURenderTarget>(_wgpuDevice, props);
#else
    return std::make_unique<RenderTexture>(props); // legacy WebGL2
#endif
#else
    return std::make_unique<SDLGPURenderTarget>(_sdlDevice, props);
#endif
}

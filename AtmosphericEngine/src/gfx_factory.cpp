#include "gfx_factory.hpp"

#ifdef __EMSCRIPTEN__
#if defined(AE_WEB_BACKEND_WEBGPU)
#include <webgpu/webgpu.h>
#include "webgpu_buffer.hpp"
#include "webgpu_render_target.hpp"
#else
#include "render_mesh.hpp"
#include "render_texture.hpp"
#endif
#else // native
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL.h>
#include "sdl_gpu_buffer.hpp"
#include "sdl_gpu_render_target.hpp"
#include "render_mesh.hpp"
#include "render_texture.hpp"
#endif

// ── Static member definitions ────────────────────────────────────────────────
GfxBackend GfxFactory::_backend = GfxBackend::OpenGL;

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
    // Device arrives async — caller must invoke SetWebGPUDevice() from the
    // emscripten_webgpu_init_default_device() callback before creating resources.
    //
    // Fallback: if SetWebGPUDevice() is never called (browser lacks WebGPU),
    // CreateBuffer() / CreateRenderTarget() automatically return GL wrappers.
}

void GfxFactory::SetWebGPUDevice(WGPUDevice device) {
#if defined(AE_WEB_BACKEND_WEBGPU)
    if (!device) {
        // WebGPU device request failed — fall back to WebGL2.
        _backend = GfxBackend::OpenGL;
        return;
    }
    _wgpuDevice = device;
    _wgpuQueue  = wgpuDeviceGetQueue(device);
#endif
}

#else // native

void GfxFactory::Init(SDL_Window* sdlWindow) {
    // Attempt SDL3 GPU (Vulkan / Metal / D3D12) first.
    // Advertise all shader formats so SDL picks the best driver available.
    const SDL_GPUShaderFormat formats =
        SDL_GPU_SHADERFORMAT_SPIRV    |
        SDL_GPU_SHADERFORMAT_DXBC     |
        SDL_GPU_SHADERFORMAT_DXIL     |
        SDL_GPU_SHADERFORMAT_MSL      |
        SDL_GPU_SHADERFORMAT_METALLIB;

    _sdlDevice = SDL_CreateGPUDevice(formats, /*debug=*/false, /*name=*/nullptr);

    if (!_sdlDevice) {
        // SDL3 GPU unavailable on this system — fall back to OpenGL.
        _backend = GfxBackend::OpenGL;
        return;
    }

    _backend = GfxBackend::SDLGPU;
    if (sdlWindow) {
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
    if (_backend == GfxBackend::WebGPU && _wgpuDevice) {
        return std::make_unique<WebGPUBuffer>(_wgpuDevice, _wgpuQueue);
    }
#endif
    // WebGL2 fallback (no WebGPU device, or AE_WEB_BACKEND_WEBGPU is OFF).
    return std::make_unique<RenderMesh>();
#else
    if (_backend == GfxBackend::SDLGPU && _sdlDevice) {
        return std::make_unique<SDLGPUBuffer>(_sdlDevice);
    }
    // OpenGL fallback (SDL3 GPU unavailable or Init() not yet called).
    return std::make_unique<RenderMesh>();
#endif
}

std::unique_ptr<IGPURenderTarget> GfxFactory::CreateRenderTarget(
        const IGPURenderTarget::Props& props) {
#ifdef __EMSCRIPTEN__
#if defined(AE_WEB_BACKEND_WEBGPU)
    if (_backend == GfxBackend::WebGPU && _wgpuDevice) {
        return std::make_unique<WebGPURenderTarget>(_wgpuDevice, props);
    }
#endif
    return std::make_unique<RenderTexture>(props);
#else
    if (_backend == GfxBackend::SDLGPU && _sdlDevice) {
        return std::make_unique<SDLGPURenderTarget>(_sdlDevice, props);
    }
    return std::make_unique<RenderTexture>(props);
#endif
}

#pragma once
#include "gpu_buffer.hpp"
#include "gpu_render_target.hpp"
#include "window.hpp"
#include <memory>

// Platform-specific headers — kept behind feature guards.
#ifndef __EMSCRIPTEN__
struct SDL_Window;
struct SDL_GPUDevice;
#endif
#if defined(AE_WEB_BACKEND_WEBGPU) && defined(__EMSCRIPTEN__)
#include <webgpu/webgpu.h>
#endif

// Static factory — call Init() once after the window is ready, then use
// CreateBuffer() / CreateRenderTarget() everywhere that needs a GPU resource.
//
// Backend assignment:
//   Native  : SDL3 GPU  (Vulkan / Metal / D3D12 chosen automatically by SDL)
//   Web     : WebGPU    (Emscripten + AE_WEB_BACKEND_WEBGPU=ON)
//
// Calling Init() on native:
//   GfxFactory::Init(sdlWindow);   // SDL_GPUDevice created and claims the window
//
// Calling Init() on web (device arrives asynchronously):
//   GfxFactory::Init();
//   // ... inside emscripten_webgpu_init_default_device() callback:
//   GfxFactory::SetWebGPUDevice(device);
class GfxFactory {
public:
#ifdef __EMSCRIPTEN__
    // Web: backend is always WebGPU. Device arrives via SetWebGPUDevice().
    static void Init();
    // Called from the emscripten_webgpu_init_default_device() callback once the
    // async device request resolves.
    static void SetWebGPUDevice(WGPUDevice device);
    static WGPUDevice GetWebGPUDevice() { return _wgpuDevice; }
    static WGPUQueue  GetWebGPUQueue()  { return _wgpuQueue; }
#else
    // Native: backend is always SDL3 GPU. Optionally pass the SDL window so
    // SDL_ClaimWindowForGPUDevice() is called immediately.
    static void Init(SDL_Window* sdlWindow = nullptr);
    static SDL_GPUDevice* GetSDLDevice() { return _sdlDevice; }
#endif

    static void Shutdown();

    static GfxBackend GetBackend() { return _backend; }

    static std::unique_ptr<IGPUBuffer>       CreateBuffer();
    static std::unique_ptr<IGPURenderTarget> CreateRenderTarget(const IGPURenderTarget::Props& props);

private:
    static GfxBackend _backend;

#ifdef __EMSCRIPTEN__
    static WGPUDevice _wgpuDevice;
    static WGPUQueue  _wgpuQueue;
#else
    static SDL_GPUDevice* _sdlDevice;
#endif
};

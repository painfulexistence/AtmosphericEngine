#pragma once
#include "gpu_buffer.hpp"
#include "gpu_render_target.hpp"
#include "window.hpp"
#include <memory>

#ifndef __EMSCRIPTEN__
struct SDL_Window;  // Needed for native window handle (Dawn surface creation)
#endif
#if defined(AE_WEB_BACKEND_WEBGPU) && defined(__EMSCRIPTEN__)
#include <webgpu/webgpu.h>
#endif

// Static factory — call Init() once after the window is ready, then use
// CreateBuffer() / CreateRenderTarget() everywhere that needs a GPU resource.
//
// Backend selection:
//   Native  : Dawn WebGPU (not yet integrated — currently falls back to OpenGL)
//   Web     : browser WebGPU via Emscripten  (AE_WEB_BACKEND_WEBGPU=ON)
//             or WebGL 2.0 fallback          (AE_WEB_BACKEND_WEBGPU=OFF)
//
// OpenGL is always the automatic fallback when the primary backend fails
// to initialize (missing driver support, Dawn not yet linked, etc.).
class GfxFactory {
public:
#ifdef __EMSCRIPTEN__
    // Web: call once at startup. Device arrives async via SetWebGPUDevice().
    static void Init();
    // Called from the emscripten_webgpu_init_default_device() callback.
    // Passing nullptr triggers an automatic fall back to WebGL 2.
    static void SetWebGPUDevice(WGPUDevice device);
    static WGPUDevice GetWebGPUDevice() { return _wgpuDevice; }
    static WGPUQueue  GetWebGPUQueue()  { return _wgpuQueue; }
#else
    // Native: sdlWindow is stored for future Dawn surface creation.
    // Currently falls back to OpenGL until Dawn is integrated.
    static void Init(SDL_Window* sdlWindow = nullptr);
#endif

    static void Shutdown();

    static GfxBackend GetBackend() { return _backend; }

    static std::unique_ptr<IGPUBuffer>       CreateBuffer();
    static std::unique_ptr<IGPURenderTarget> CreateRenderTarget(const IGPURenderTarget::Props& props);

private:
    static GfxBackend  _backend;

#ifdef __EMSCRIPTEN__
    static WGPUDevice  _wgpuDevice;
    static WGPUQueue   _wgpuQueue;
#else
    static SDL_Window* _sdlWindow;  // Retained for Dawn surface creation
#endif
};

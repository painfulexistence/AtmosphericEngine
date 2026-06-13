#pragma once
#include "buffer.hpp"
#include "render_target.hpp"
#include "window.hpp"
#include <memory>

#ifndef __EMSCRIPTEN__
struct SDL_Window;
#endif
#if defined(AE_USE_WEBGPU) && defined(__EMSCRIPTEN__)
#include <webgpu/webgpu.h>
#endif

// Static factory — call Init() once after the window is ready, then use
// CreateBuffer() / CreateRenderTarget() everywhere that needs a GPU resource.
//
// AE_USE_WEBGPU (CMake option, default OFF):
//   Controls whether WebGPU support is compiled in.
//   The actual backend is chosen at runtime:
//     - WebGPU support compiled in AND browser/device reports availability
//       → attempts WebGPU; falls back to OpenGL/WebGL 2 on failure
//     - WebGPU support not compiled in, OR unavailable at runtime
//       → OpenGL 4.1 (native) / WebGL 2 (Emscripten)
class GfxFactory {
public:
#ifdef __EMSCRIPTEN__
    // Web: checks WebGPU availability at runtime; falls back to WebGL 2.
    static void Init();
#if defined(AE_USE_WEBGPU)
    // Called from the emscripten_webgpu_get_device() callback.
    // Passing nullptr falls back to WebGL 2.
    static void SetWebGPUDevice(WGPUDevice device);
    static WGPUDevice GetWebGPUDevice() { return _wgpuDevice; }
    static WGPUQueue  GetWebGPUQueue()  { return _wgpuQueue; }
#endif
#else
    // Native: stores sdlWindow for future Dawn surface creation.
    // Currently always falls back to OpenGL until Dawn is integrated.
    static void Init(SDL_Window* sdlWindow = nullptr);
#endif

    static void Shutdown();

    static GfxBackend GetBackend() { return _backend; }

    static std::unique_ptr<Buffer>       CreateBuffer();
    static std::unique_ptr<RenderTarget> CreateRenderTarget(const RenderTarget::Props& props);

private:
    static GfxBackend _backend;

#if defined(__EMSCRIPTEN__) && defined(AE_USE_WEBGPU)
    static WGPUDevice _wgpuDevice;
    static WGPUQueue  _wgpuQueue;
#elif !defined(__EMSCRIPTEN__)
    static SDL_Window* _sdlWindow;
#endif
};

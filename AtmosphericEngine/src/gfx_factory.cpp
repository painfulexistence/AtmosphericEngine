#include "gfx_factory.hpp"
#include "gl_buffer.hpp"
#include "gl_render_target.hpp"
#include "console.hpp"

#if defined(AE_USE_WEBGPU) && defined(__EMSCRIPTEN__)
#include <webgpu/webgpu.h>
#include "gpu_buffer.hpp"
#include "gpu_render_target.hpp"
#endif

// ── Static member definitions ────────────────────────────────────────────────
GfxBackend GfxFactory::_backend = GfxBackend::OpenGL;

#if defined(__EMSCRIPTEN__) && defined(AE_USE_WEBGPU)
WGPUDevice GfxFactory::_wgpuDevice = nullptr;
WGPUQueue  GfxFactory::_wgpuQueue  = nullptr;
#elif !defined(__EMSCRIPTEN__)
SDL_Window* GfxFactory::_sdlWindow = nullptr;
#endif

// ── Init ─────────────────────────────────────────────────────────────────────
#ifdef __EMSCRIPTEN__

void GfxFactory::Init() {
#if defined(AE_USE_WEBGPU)
    if (Window::IsWebGPUAvailable()) {
        _backend = GfxBackend::WebGPU;
        // Device arrives async — caller must invoke SetWebGPUDevice() once
        // emscripten_webgpu_get_device() callback fires.
        return;
    } else {
        Console::Get()->Warn("[GfxFactory] WebGPU is not supported or enabled in the browser. Falling back to WebGL 2.0 (OpenGL ES3).");
    }
#endif
    _backend = GfxBackend::OpenGL;  // No WebGPU support or unavailable → WebGL 2
}

#if defined(AE_USE_WEBGPU)
void GfxFactory::SetWebGPUDevice(WGPUDevice device) {
    if (!device) {
        Console::Get()->Warn("[GfxFactory] Failed to request WebGPU device asynchronously. Falling back to WebGL 2.0 (OpenGL ES3).");
        _backend = GfxBackend::OpenGL;  // device creation failed → WebGL 2
        return;
    }
    _wgpuDevice = device;
    _wgpuQueue  = wgpuDeviceGetQueue(device);
}
#endif

#else // native

void GfxFactory::Init(SDL_Window* sdlWindow) {
    _sdlWindow = sdlWindow;

    // TODO: Initialize Dawn WebGPU here.
    //
    // Example flow once Dawn is added as a CMake dependency:
    //
    //   WGPUInstanceDescriptor instDesc{};
    //   WGPUInstance instance = wgpuCreateInstance(&instDesc);
    //
    //   WGPUSurface surface = CreateDawnSurface(instance, sdlWindow);
    //
    //   WGPUDevice device = RequestDawnDevice(instance, surface);
    //   if (!device) { _backend = GfxBackend::OpenGL; return; }
    //
    //   _backend = GfxBackend::WebGPU;
    //   return;

    _backend = GfxBackend::OpenGL;
}

#endif

// ── Shutdown ─────────────────────────────────────────────────────────────────
void GfxFactory::Shutdown() {
#if defined(__EMSCRIPTEN__) && defined(AE_USE_WEBGPU)
    if (_wgpuDevice) {
        wgpuDeviceRelease(_wgpuDevice);
        _wgpuDevice = nullptr;
        _wgpuQueue  = nullptr;
    }
#elif !defined(__EMSCRIPTEN__)
    _sdlWindow = nullptr;
    // TODO: release Dawn instance / device / surface here.
#endif
}

// ── Factory methods ──────────────────────────────────────────────────────────
std::unique_ptr<Buffer> GfxFactory::CreateBuffer() {
#if defined(AE_USE_WEBGPU) && defined(__EMSCRIPTEN__)
    if (_backend == GfxBackend::WebGPU && _wgpuDevice)
        return std::make_unique<GPUBuffer>(_wgpuDevice, _wgpuQueue);
#elif !defined(__EMSCRIPTEN__)
    // TODO: return DawnGPUBuffer(_dawnDevice) once Dawn is integrated.
    (void)_sdlWindow;
#endif
    return std::make_unique<GLBuffer>();
}

std::unique_ptr<RenderTarget> GfxFactory::CreateRenderTarget(
        const RenderTarget::Props& props) {
#if defined(AE_USE_WEBGPU) && defined(__EMSCRIPTEN__)
    if (_backend == GfxBackend::WebGPU && _wgpuDevice)
        return std::make_unique<GPURenderTarget>(_wgpuDevice, props);
#elif !defined(__EMSCRIPTEN__)
    // TODO: return DawnGPURenderTarget(_dawnDevice, props) once Dawn is integrated.
    (void)_sdlWindow;
#endif
    return std::make_unique<GLRenderTarget>(props);
}

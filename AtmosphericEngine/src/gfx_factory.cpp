#include "gfx_factory.hpp"
#include "gl_buffer.hpp"
#include "gl_render_target.hpp"

#ifdef __EMSCRIPTEN__
#if defined(AE_WEB_BACKEND_WEBGPU)
#include <webgpu/webgpu.h>
#include "gpu_buffer.hpp"
#include "gpu_render_target.hpp"
#endif
#endif

// ── Static member definitions ────────────────────────────────────────────────
GfxBackend GfxFactory::_backend = GfxBackend::OpenGL;

#ifdef __EMSCRIPTEN__
WGPUDevice GfxFactory::_wgpuDevice = nullptr;
WGPUQueue  GfxFactory::_wgpuQueue  = nullptr;
#else
SDL_Window* GfxFactory::_sdlWindow = nullptr;
#endif

// ── Init ─────────────────────────────────────────────────────────────────────
#ifdef __EMSCRIPTEN__

void GfxFactory::Init() {
    _backend = GfxBackend::WebGPU;
    // Device arrives async. Caller must invoke SetWebGPUDevice() once
    // emscripten_webgpu_get_device() callback fires.
}

void GfxFactory::SetWebGPUDevice(WGPUDevice device) {
#if defined(AE_WEB_BACKEND_WEBGPU)
    if (!device) {
        _backend = GfxBackend::OpenGL;  // browser lacks WebGPU — fall back to WebGL 2
        return;
    }
    _wgpuDevice = device;
    _wgpuQueue  = wgpuDeviceGetQueue(device);
#endif
}

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
#ifdef __EMSCRIPTEN__
#if defined(AE_WEB_BACKEND_WEBGPU)
    if (_wgpuDevice) {
        wgpuDeviceRelease(_wgpuDevice);
        _wgpuDevice = nullptr;
        _wgpuQueue  = nullptr;
    }
#endif
#else
    _sdlWindow = nullptr;
    // TODO: release Dawn instance / device / surface here.
#endif
}

// ── Factory methods ──────────────────────────────────────────────────────────
std::unique_ptr<Buffer> GfxFactory::CreateBuffer() {
#ifdef __EMSCRIPTEN__
#if defined(AE_WEB_BACKEND_WEBGPU)
    if (_backend == GfxBackend::WebGPU && _wgpuDevice)
        return std::make_unique<GPUBuffer>(_wgpuDevice, _wgpuQueue);
#endif
#else
    // TODO: return DawnGPUBuffer(_dawnDevice) once Dawn is integrated.
    (void)_sdlWindow;
#endif
    return std::make_unique<GLBuffer>();
}

std::unique_ptr<RenderTarget> GfxFactory::CreateRenderTarget(
        const RenderTarget::Props& props) {
#ifdef __EMSCRIPTEN__
#if defined(AE_WEB_BACKEND_WEBGPU)
    if (_backend == GfxBackend::WebGPU && _wgpuDevice)
        return std::make_unique<GPURenderTarget>(_wgpuDevice, props);
#endif
#else
    // TODO: return DawnGPURenderTarget(_dawnDevice, props) once Dawn is integrated.
    (void)_sdlWindow;
#endif
    return std::make_unique<GLRenderTarget>(props);
}

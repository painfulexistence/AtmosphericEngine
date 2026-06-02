#pragma once
#include "gpu_buffer.hpp"
#include "gpu_render_target.hpp"
#include "window.hpp"
#include <memory>

#ifndef __EMSCRIPTEN__
// Forward-declare SDL types to keep this header free of SDL includes.
struct SDL_Window;
struct SDL_GPUDevice;
#endif

// Static factory that creates the correct IGPUBuffer / IGPURenderTarget
// implementation for the active graphics backend.
//
// Usage:
//   // At startup, after creating the OS window:
//   GfxFactory::Init(GfxBackend::OpenGL);              // existing GL path
//   GfxFactory::Init(GfxBackend::OpenGL, sdlWindow);   // native SDL3 GPU path (future)
//
//   // Everywhere else:
//   auto buf = GfxFactory::CreateBuffer();
//   auto rt  = GfxFactory::CreateRenderTarget({ .width=1920, .height=1080, .withDepth=true });
class GfxFactory {
public:
#ifdef __EMSCRIPTEN__
    static void Init(GfxBackend backend);
#else
    // sdlWindow is required when backend != OpenGL so the GPU device can claim it.
    static void Init(GfxBackend backend, SDL_Window* sdlWindow = nullptr);
    static SDL_GPUDevice* GetSDLDevice() { return _sdlDevice; }
#endif
    static void Shutdown();

    static GfxBackend GetBackend() { return _backend; }

    static std::unique_ptr<IGPUBuffer>       CreateBuffer();
    static std::unique_ptr<IGPURenderTarget> CreateRenderTarget(const IGPURenderTarget::Props& props);

private:
    static GfxBackend _backend;
#ifndef __EMSCRIPTEN__
    static SDL_GPUDevice* _sdlDevice;
#endif
};

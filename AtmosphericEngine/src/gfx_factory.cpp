#include "gfx_factory.hpp"
#include "render_mesh.hpp"
#include "render_texture.hpp"

#ifndef __EMSCRIPTEN__
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL.h>
#include "sdl_gpu_buffer.hpp"
#include "sdl_gpu_render_target.hpp"
#endif

// ── Static member definitions ────────────────────────────────────────────────
GfxBackend GfxFactory::_backend = GfxBackend::OpenGL;
#ifndef __EMSCRIPTEN__
SDL_GPUDevice* GfxFactory::_sdlDevice = nullptr;
#endif

// ── Init / Shutdown ──────────────────────────────────────────────────────────
#ifdef __EMSCRIPTEN__
void GfxFactory::Init(GfxBackend backend) {
    _backend = backend;
    // Emscripten always runs the WebGL2 path; no extra device to create.
}
#else
void GfxFactory::Init(GfxBackend backend, SDL_Window* sdlWindow) {
    _backend = backend;

    if (backend == GfxBackend::OpenGL) return;

    // Request all shader formats so SDL3 picks the best available driver
    // (Vulkan on Linux/Windows, Metal on macOS, D3D12 on Windows).
    const SDL_GPUShaderFormat formats =
        SDL_GPU_SHADERFORMAT_SPIRV    |
        SDL_GPU_SHADERFORMAT_DXBC     |
        SDL_GPU_SHADERFORMAT_DXIL     |
        SDL_GPU_SHADERFORMAT_MSL      |
        SDL_GPU_SHADERFORMAT_METALLIB;

    _sdlDevice = SDL_CreateGPUDevice(formats, /*debug=*/false, /*name=*/nullptr);
    if (!_sdlDevice) {
        // Driver doesn't support SDL3 GPU — fall back to OpenGL transparently.
        _backend = GfxBackend::OpenGL;
        return;
    }
    if (sdlWindow) {
        SDL_ClaimWindowForGPUDevice(_sdlDevice, sdlWindow);
    }
}
#endif

void GfxFactory::Shutdown() {
#ifndef __EMSCRIPTEN__
    if (_sdlDevice) {
        SDL_DestroyGPUDevice(_sdlDevice);
        _sdlDevice = nullptr;
    }
#endif
}

// ── Factory methods ──────────────────────────────────────────────────────────
std::unique_ptr<IGPUBuffer> GfxFactory::CreateBuffer() {
#ifndef __EMSCRIPTEN__
    if (_backend != GfxBackend::OpenGL && _sdlDevice) {
        return std::make_unique<SDLGPUBuffer>(_sdlDevice);
    }
#endif
    return std::make_unique<RenderMesh>();
}

std::unique_ptr<IGPURenderTarget> GfxFactory::CreateRenderTarget(
        const IGPURenderTarget::Props& props) {
#ifndef __EMSCRIPTEN__
    if (_backend != GfxBackend::OpenGL && _sdlDevice) {
        return std::make_unique<SDLGPURenderTarget>(_sdlDevice, props);
    }
#endif
    return std::make_unique<RenderTexture>(props);
}

#pragma once

// Native SDL3 GPU types
#ifndef __EMSCRIPTEN__
#include <SDL3/SDL_gpu.h>
#endif

// Browser WebGPU C API (available when compiled with Emscripten + USE_WEBGPU,
// or with Dawn as a standalone library)
#if defined(AE_WEB_BACKEND_WEBGPU) && defined(__EMSCRIPTEN__)
#include <webgpu/webgpu.h>
#endif

// Per-frame GPU recording context threaded through Draw() and Begin() calls.
//
// OpenGL is an implicit-state API — no explicit command recording.
// GLCommandContext is therefore empty; pass nullptr or a default instance.
//
// For explicit-state APIs (SDL3 GPU, WebGPU) the context carries the live
// command buffer / render pass handles for the current frame.
class IGPUCommandContext {
public:
    virtual ~IGPUCommandContext() = default;
};

// OpenGL: no recording state needed.
class GLCommandContext : public IGPUCommandContext {};

// SDL3 GPU per-frame recording handles.
// Populate before calling Begin() / Draw():
//   sdlCtx.cmdBuf = SDL_AcquireGPUCommandBuffer(device);
//   // Begin() sets sdlCtx.renderPass via SDL_BeginGPURenderPass.
#ifndef __EMSCRIPTEN__
struct SDLGPUCommandContext : public IGPUCommandContext {
    SDL_GPUCommandBuffer* cmdBuf    = nullptr;
    SDL_GPURenderPass*    renderPass = nullptr;
};
#endif

// WebGPU per-frame recording handles (Emscripten + AE_WEB_BACKEND_WEBGPU).
// Populate before calling Begin() / Draw():
//   wgpuCtx.encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
//   // Begin() sets wgpuCtx.pass via wgpuCommandEncoderBeginRenderPass.
#if defined(AE_WEB_BACKEND_WEBGPU) && defined(__EMSCRIPTEN__)
struct WebGPUCommandContext : public IGPUCommandContext {
    WGPUCommandEncoder    encoder = nullptr;
    WGPURenderPassEncoder pass    = nullptr;
};
#endif

#pragma once
#ifndef __EMSCRIPTEN__
#include <SDL3/SDL_gpu.h>
#endif

// Per-frame GPU recording context threaded through Draw() and Begin() calls.
//
// OpenGL is an implicit-state API — no explicit command recording.
// GLCommandContext is therefore empty; pass nullptr or a default instance.
//
// SDL3 GPU uses explicit command buffers and render passes. Before issuing
// any draw calls, populate SDLGPUCommandContext with the live handles for
// the current frame.
class IGPUCommandContext {
public:
    virtual ~IGPUCommandContext() = default;
};

// OpenGL: no recording state needed.
class GLCommandContext : public IGPUCommandContext {};

#ifndef __EMSCRIPTEN__
// SDL3 GPU per-frame recording handles.
// Fill these in before calling Begin() / Draw():
//   sdlCtx.cmdBuf = SDL_AcquireGPUCommandBuffer(device);
//   // Begin() will set sdlCtx.renderPass after SDL_BeginGPURenderPass.
struct SDLGPUCommandContext : public IGPUCommandContext {
    SDL_GPUCommandBuffer* cmdBuf    = nullptr;
    SDL_GPURenderPass*    renderPass = nullptr;
};
#endif

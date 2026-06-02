#pragma once

// Browser WebGPU C API — available when Emscripten + AE_WEB_BACKEND_WEBGPU.
// Future: also included for native Dawn path once integrated.
#if defined(AE_WEB_BACKEND_WEBGPU) && defined(__EMSCRIPTEN__)
#include <webgpu/webgpu.h>
#endif

// Per-frame GPU recording context threaded through Draw() and Begin() calls.
//
// OpenGL is an implicit-state API — no explicit command recording.
// GLCommandContext is therefore empty; pass nullptr or a default instance.
//
// WebGPU (Dawn or browser) uses explicit command encoders and render pass
// encoders. Populate before calling Begin() / Draw():
//   ctx.encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
//   // Begin() sets ctx.pass via wgpuCommandEncoderBeginRenderPass.
class IGPUCommandContext {
public:
    virtual ~IGPUCommandContext() = default;
};

// OpenGL: no recording state needed.
class GLCommandContext : public IGPUCommandContext {};

// WebGPU per-frame recording handles.
// The same struct is used for both native Dawn and Emscripten browser WebGPU
// since they share the wgpu* C API.
#if defined(AE_WEB_BACKEND_WEBGPU) && defined(__EMSCRIPTEN__)
struct WebGPUCommandContext : public IGPUCommandContext {
    WGPUCommandEncoder    encoder = nullptr;
    WGPURenderPassEncoder pass    = nullptr;
};
#endif
// TODO: add native Dawn WebGPUCommandContext here (same struct, different guard)
// once Dawn is integrated as a CMake dependency.

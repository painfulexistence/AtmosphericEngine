#pragma once
#ifndef __EMSCRIPTEN__
#include "gpu_buffer.hpp"
#include "gpu_command_context.hpp"
#include <SDL3/SDL_gpu.h>

// SDL3 GPU implementation of IGPUBuffer.
//
// Upload path:  creates a one-shot copy command buffer per Upload() call.
//               For production use, batch uploads into a single command buffer
//               at the start of the frame.
//
// Draw path:    binds vertex/index buffers and issues draw calls through the
//               SDLGPUCommandContext's active render pass.
//               Topology is encoded in the SDL_GPUGraphicsPipeline at creation
//               time; the topology parameter in Draw() is therefore advisory only.
class SDLGPUBuffer : public IGPUBuffer {
public:
    explicit SDLGPUBuffer(SDL_GPUDevice* device);
    ~SDLGPUBuffer() override;

    SDLGPUBuffer(const SDLGPUBuffer&) = delete;
    SDLGPUBuffer& operator=(const SDLGPUBuffer&) = delete;

    void Initialize(VertexFormat format, BufferUsage usage = BufferUsage::Static) override;
    void Upload(const void* vertexData, size_t vertexCount, size_t vertexSize) override;
    void Upload(
        const void* vertexData, size_t vertexCount, size_t vertexSize,
        const uint16_t* indexData, size_t indexCount) override;
    void Draw(
        IGPUCommandContext* ctx = nullptr,
        PrimitiveTopology topology = PrimitiveTopology::Triangles) const override;

    bool IsInitialized() const override { return _initialized; }
    size_t GetVertexCount() const override { return _vertexCount; }
    size_t GetIndexCount() const override { return _indexCount; }
    VertexFormat GetFormat() const override { return _format; }

private:
    void UploadToGPU(const void* data, size_t bytes,
                     SDL_GPUBufferUsageFlags usage, SDL_GPUBuffer** outBuf);

    SDL_GPUDevice* _device       = nullptr;
    SDL_GPUBuffer* _vertexBuffer = nullptr;
    SDL_GPUBuffer* _indexBuffer  = nullptr;

    VertexFormat _format    = VertexFormat::Standard;
    size_t _vertexCount     = 0;
    size_t _indexCount      = 0;
    bool _initialized       = false;
    bool _hasIndices        = false;
};
#endif // !__EMSCRIPTEN__

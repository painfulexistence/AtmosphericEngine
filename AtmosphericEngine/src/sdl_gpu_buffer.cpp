#ifndef __EMSCRIPTEN__
#include "sdl_gpu_buffer.hpp"
#include <cstring>

SDLGPUBuffer::SDLGPUBuffer(SDL_GPUDevice* device) : _device(device) {}

SDLGPUBuffer::~SDLGPUBuffer() {
    if (_vertexBuffer) SDL_ReleaseGPUBuffer(_device, _vertexBuffer);
    if (_indexBuffer)  SDL_ReleaseGPUBuffer(_device, _indexBuffer);
}

void SDLGPUBuffer::Initialize(VertexFormat format, BufferUsage /*usage*/) {
    _format = format;
    _initialized = true;
    // SDL3 GPU buffers are allocated in UploadToGPU(); nothing to do here.
}

void SDLGPUBuffer::UploadToGPU(const void* data, size_t bytes,
                                SDL_GPUBufferUsageFlags usage, SDL_GPUBuffer** outBuf) {
    // Release any previous allocation.
    if (*outBuf) {
        SDL_ReleaseGPUBuffer(_device, *outBuf);
        *outBuf = nullptr;
    }

    // Allocate the device-local buffer.
    SDL_GPUBufferCreateInfo bufInfo{};
    bufInfo.usage = usage;
    bufInfo.size  = static_cast<Uint32>(bytes);
    *outBuf = SDL_CreateGPUBuffer(_device, &bufInfo);

    // Allocate a host-visible transfer buffer and copy the data into it.
    SDL_GPUTransferBufferCreateInfo transInfo{};
    transInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transInfo.size  = static_cast<Uint32>(bytes);
    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(_device, &transInfo);

    void* mapped = SDL_MapGPUTransferBuffer(_device, transfer, false);
    std::memcpy(mapped, data, bytes);
    SDL_UnmapGPUTransferBuffer(_device, transfer);

    // Record and submit the upload in a one-shot command buffer.
    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(_device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuf);

    SDL_GPUTransferBufferLocation src{};
    src.transfer_buffer = transfer;
    src.offset = 0;

    SDL_GPUBufferRegion dst{};
    dst.buffer = *outBuf;
    dst.offset = 0;
    dst.size   = static_cast<Uint32>(bytes);

    SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cmdBuf);
    SDL_ReleaseGPUTransferBuffer(_device, transfer);
}

void SDLGPUBuffer::Upload(const void* vertexData, size_t vertexCount, size_t vertexSize) {
    if (!_initialized) Initialize(_format);
    _vertexCount = vertexCount;
    _hasIndices  = false;
    UploadToGPU(vertexData, vertexCount * vertexSize, SDL_GPU_BUFFERUSAGE_VERTEX, &_vertexBuffer);
}

void SDLGPUBuffer::Upload(const void* vertexData, size_t vertexCount, size_t vertexSize,
                           const uint16_t* indexData, size_t indexCount) {
    if (!_initialized) Initialize(_format);
    _vertexCount = vertexCount;
    _indexCount  = indexCount;
    _hasIndices  = true;
    UploadToGPU(vertexData, vertexCount * vertexSize,        SDL_GPU_BUFFERUSAGE_VERTEX, &_vertexBuffer);
    UploadToGPU(indexData,  indexCount  * sizeof(uint16_t),  SDL_GPU_BUFFERUSAGE_INDEX,  &_indexBuffer);
}

void SDLGPUBuffer::Draw(IGPUCommandContext* ctx, PrimitiveTopology /*topology*/) const {
    // topology is encoded in the SDL_GPUGraphicsPipeline bound before this call.
    auto* sdlCtx = static_cast<SDLGPUCommandContext*>(ctx);
    SDL_GPURenderPass* pass = sdlCtx->renderPass;

    SDL_GPUBufferBinding vb{};
    vb.buffer = _vertexBuffer;
    vb.offset = 0;
    SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

    if (_hasIndices) {
        SDL_GPUBufferBinding ib{};
        ib.buffer = _indexBuffer;
        ib.offset = 0;
        SDL_BindGPUIndexBuffer(pass, &ib, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        SDL_DrawGPUIndexedPrimitives(pass, static_cast<Uint32>(_indexCount), 1, 0, 0, 0);
    } else {
        SDL_DrawGPUPrimitives(pass, static_cast<Uint32>(_vertexCount), 1, 0, 0);
    }
}
#endif // !__EMSCRIPTEN__

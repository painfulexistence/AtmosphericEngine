#if defined(AE_WEB_BACKEND_WEBGPU) && defined(__EMSCRIPTEN__)
#include "webgpu_buffer.hpp"

WebGPUBuffer::WebGPUBuffer(WGPUDevice device, WGPUQueue queue)
    : _device(device), _queue(queue) {}

WebGPUBuffer::~WebGPUBuffer() {
    if (_vertexBuffer) { wgpuBufferRelease(_vertexBuffer); _vertexBuffer = nullptr; }
    if (_indexBuffer)  { wgpuBufferRelease(_indexBuffer);  _indexBuffer  = nullptr; }
}

void WebGPUBuffer::Initialize(VertexFormat format, BufferUsage /*usage*/) {
    _format = format;
    _initialized = true;
    // WebGPU buffers are allocated in AllocAndUpload(); nothing to do here.
}

WGPUBuffer WebGPUBuffer::AllocAndUpload(const void* data, size_t bytes,
                                         WGPUBufferUsageFlags usage) {
    WGPUBufferDescriptor desc{};
    desc.usage            = usage | WGPUBufferUsage_CopyDst;
    desc.size             = bytes;
    desc.mappedAtCreation = false;

    WGPUBuffer buf = wgpuDeviceCreateBuffer(_device, &desc);
    // wgpuQueueWriteBuffer is the simplest synchronous upload path on web.
    wgpuQueueWriteBuffer(_queue, buf, 0, data, bytes);
    return buf;
}

void WebGPUBuffer::Upload(const void* vertexData, size_t vertexCount, size_t vertexSize) {
    if (!_initialized) Initialize(_format);
    if (_vertexBuffer) { wgpuBufferRelease(_vertexBuffer); _vertexBuffer = nullptr; }
    _vertexCount = vertexCount;
    _hasIndices  = false;
    _vertexBuffer = AllocAndUpload(vertexData, vertexCount * vertexSize, WGPUBufferUsage_Vertex);
}

void WebGPUBuffer::Upload(const void* vertexData, size_t vertexCount, size_t vertexSize,
                           const uint16_t* indexData, size_t indexCount) {
    if (!_initialized) Initialize(_format);
    if (_vertexBuffer) { wgpuBufferRelease(_vertexBuffer); _vertexBuffer = nullptr; }
    if (_indexBuffer)  { wgpuBufferRelease(_indexBuffer);  _indexBuffer  = nullptr; }
    _vertexCount = vertexCount;
    _indexCount  = indexCount;
    _hasIndices  = true;
    _vertexBuffer = AllocAndUpload(vertexData, vertexCount * vertexSize,       WGPUBufferUsage_Vertex);
    _indexBuffer  = AllocAndUpload(indexData,  indexCount  * sizeof(uint16_t), WGPUBufferUsage_Index);
}

void WebGPUBuffer::Draw(IGPUCommandContext* ctx, PrimitiveTopology /*topology*/) const {
    // topology is encoded in the WGPURenderPipeline bound before this call.
    auto* wgpuCtx = static_cast<WebGPUCommandContext*>(ctx);
    WGPURenderPassEncoder pass = wgpuCtx->pass;

    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, _vertexBuffer, 0, WGPU_WHOLE_SIZE);

    if (_hasIndices) {
        wgpuRenderPassEncoderSetIndexBuffer(
            pass, _indexBuffer, WGPUIndexFormat_Uint16, 0, WGPU_WHOLE_SIZE);
        wgpuRenderPassEncoderDrawIndexed(
            pass, static_cast<uint32_t>(_indexCount), 1, 0, 0, 0);
    } else {
        wgpuRenderPassEncoderDraw(pass, static_cast<uint32_t>(_vertexCount), 1, 0, 0);
    }
}
#endif // AE_WEB_BACKEND_WEBGPU && __EMSCRIPTEN__

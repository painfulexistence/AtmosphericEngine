// Injected via --pre-js when AE_USE_WEBGPU=ON.
//
// Requests a WebGPU adapter + device asynchronously before the WASM module
// starts.  Module.addRunDependency / removeRunDependency pause Emscripten's
// runtime until the promise resolves, so emscripten_webgpu_get_device() in
// GfxFactory::Init() will always find a valid device (or null on failure).
(function () {
    var Module = Module || {};
    Module['preRun'] = Module['preRun'] || [];
    Module['preRun'].push(function () {
        Module.addRunDependency('webgpu-preinit');
        (navigator.gpu
            ? navigator.gpu.requestAdapter({ powerPreference: 'high-performance' })
            : Promise.resolve(null))
        .then(function (adapter) {
            return adapter ? adapter.requestDevice() : null;
        })
        .then(function (device) {
            if (device) {
                Module['preinitializedWebGPUDevice'] = device;
                console.log('[AtmosphericEngine] WebGPU device pre-initialised.');
            } else {
                console.warn('[AtmosphericEngine] WebGPU pre-init: no device obtained; will fall back to WebGL 2.');
            }
        })
        .catch(function (e) {
            console.error('[AtmosphericEngine] WebGPU pre-init failed:', e);
        })
        .finally(function () {
            Module.removeRunDependency('webgpu-preinit');
        });
    });
})();

// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the WebGL version of canvaskit.
// Functions in this file are supplemented by cpu.js.
(function(CanvasKit){
    CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
    CanvasKit._extraInitializations.push(function() {
      CanvasKit.MakeWebGPUThing = async function(device) {
        if (!device) {
            const suggestion = 'Check #enable-unsafe-webgpu flag if using Chromium';
            if (!navigator.gpu) {
                console.error('WebGPU is not supported in this browser. ' + suggestion);
                return null;
            }

            const adapter = await navigator.gpu.requestAdapter();
            if (!adapter) {
                console.error('No WebGPU adapter found. ' + suggestion);
                return null;
            }

            device = await adapter.requestDevice();
            if (!device) {
                console.error('No WebGPU device found. ' + suggestion);
                return null;
            }
        }

        let devPtr = WebGPU.importJsDevice(device);

        return CanvasKit.TestGraphite(devPtr);
      };
    });
}(Module));  // When this file is loaded in, the high level object is "Module".

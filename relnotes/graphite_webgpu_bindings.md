Graphite WebAssembly builds can opt into Dawn's modern emdawnwebgpu browser
bindings with `skia_use_emdawnwebgpu=true`. Emscripten's legacy WebGPU bindings
remain the default and are still required by CanvasKit's JavaScript interop.
Native Dawn builds and the browser tick-function contract are unchanged.

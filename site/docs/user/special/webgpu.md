---
title: "WebGPU browser bindings"
linkTitle: "WebGPU"

---

WebAssembly Graphite Dawn builds use Emscripten's legacy WebGPU bindings
(`-sUSE_WEBGPU=1`) by default. To use Dawn's emdawnwebgpu bindings instead, set:

```gn
target_cpu = "wasm"
skia_enable_graphite = true
skia_use_dawn = true
skia_use_emdawnwebgpu = true
```

This defines `SK_USE_EMDAWNWEBGPU` and passes `--use-port=emdawnwebgpu` when
compiling and linking, including GN clients depending on `//:skia`. To use a
downloaded package, set `skia_emdawnwebgpu_port` to the absolute path of its
`emdawnwebgpu-*.remoteport.py` or `emdawnwebgpu_pkg/emdawnwebgpu.port.py`.
See [Dawn's package documentation](https://dawn.googlesource.com/dawn/+/main/src/emdawnwebgpu/pkg/README.md)
for SDK requirements and port options.

Clients building outside GN must define `SK_USE_EMDAWNWEBGPU` and pass the same
`--use-port` when compiling and linking. Do not combine it with `-sUSE_WEBGPU=1`.

CanvasKit's existing JavaScript interop still requires the legacy bindings.
Native Dawn builds and the `DawnBackendContext` tick-function contract are
unchanged. Browser shader compilation diagnostics require a tick function.

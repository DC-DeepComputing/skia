
---
title: "Specialized Builds"
linkTitle: "Specialized Builds"

weight: 6

---

## Graphite WebGPU browser bindings

WebAssembly builds with `skia_enable_graphite=true skia_use_dawn=true` use
Emscripten's legacy WebGPU bindings by default. This preserves CanvasKit's
existing JavaScript interop and requires an Emscripten version that still
provides `-sUSE_WEBGPU=1`.

To use Dawn's modern browser bindings instead, set:

```
target_cpu = "wasm"
skia_enable_graphite = true
skia_use_dawn = true
skia_use_emdawnwebgpu = true
```

This passes `--use-port=emdawnwebgpu` during compilation and linking. Use an
Emscripten SDK containing that port, or set `skia_emdawnwebgpu_port` to the
absolute path of a downloaded `emdawnwebgpu-*.remoteport.py` or extracted
`emdawnwebgpu_pkg/emdawnwebgpu.port.py`. Remote ports require Emscripten 4.0.10
or newer; local packages can also be used with older SDKs supported by Dawn.
See [Dawn's package documentation](https://dawn.googlesource.com/dawn/+/main/src/emdawnwebgpu/pkg/README.md)
for package versions, SDK requirements, and port options.

The selection applies to all consumers of the GN Dawn dependency so that Skia
and its clients use matching headers and bindings. Clients linking a Skia static
archive outside GN must pass the same `--use-port` at their final Emscripten link
step and compile against the matching headers with `SK_USE_EMDAWNWEBGPU` defined.
Do not combine emdawnwebgpu with `-sUSE_WEBGPU=1`.

This selects the browser binding integration, not the Graphite GPU backend.
Native Dawn builds are unchanged. CanvasKit currently requires the legacy
bindings because its JavaScript interop has not been migrated.

The `DawnBackendContext` tick-function contract is unchanged. With no tick
function, submission is non-yielding, CPU-synchronous submission is disallowed,
and clients must wait for GPU completion before destroying the context.
Selecting modern bindings does not enable synchronous buffer mapping or
asynchronous pipeline compilation. Shader compilation diagnostics that require
waiting for a browser callback are only collected when a tick function is
provided.

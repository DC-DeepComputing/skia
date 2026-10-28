---
name: skia/modules/notify-reset-context-on-external-gpu-state-change
description: When client callbacks or external operations modify underlying 3D graphics state, notify Skia by calling GrDirectContext::resetContext with the corresponding backend state flags
rubric: correctness
severity: should_fix
---

**Rule.** When external operations, direct driver calls, or client callbacks
(such as WebGL or OpenGL texture generation) alter the underlying 3D graphics
API state out-of-band, notify Skia by invoking
GrDirectContext::resetContext(...) with the appropriate state flags (such as
kTextureBinding_GrGLBackendState). Do not omit resetContext when delegating
texture creation or binding to external code.

**Why.** Skia aggressively caches driver and context state (such as bound
textures and framebuffers) to eliminate redundant GPU state transitions. If
external code modifies state without notifying Skia, Skia's cached assumptions
become stale, leading to rendering corruption, texture mismatch, or driver
errors on subsequent draw calls.

## Examples

Bad:

```cpp
std::unique_ptr<GrExternalTexture> generateExternalTexture(GrRecordingContext* ctx,
                                                           GrMipMapped mipmapped) override {
    GrGLTextureInfo glInfo;
    glInfo.fID     = fCallback.call<uint32_t>("makeTexture");
    glInfo.fFormat = GR_GL_RGBA8;
    glInfo.fTarget = GR_GL_TEXTURE_2D;

    auto backendTexture = GrBackendTextures::MakeGL(fInfo.width(), fInfo.height(), mipmapped, glInfo);
    return std::make_unique<ExternalWebGLTexture>(
        backendTexture, glInfo.fID, emscripten_webgl_get_current_context());
}
```

Good:

```cpp
std::unique_ptr<GrExternalTexture> generateExternalTexture(GrRecordingContext* ctx,
                                                           GrMipMapped mipmapped) override {
    GrGLTextureInfo glInfo;
    glInfo.fID     = fCallback.call<uint32_t>("makeTexture");
    glInfo.fFormat = GR_GL_RGBA8;
    glInfo.fTarget = GR_GL_TEXTURE_2D;

    auto backendTexture = GrBackendTextures::MakeGL(fInfo.width(), fInfo.height(), mipmapped, glInfo);
    // makeTexture modified which texture is bound in the active WebGL context.
    GrAsDirectContext(ctx)->resetContext(kTextureBinding_GrGLBackendState);
    return std::make_unique<ExternalWebGLTexture>(
        backendTexture, glInfo.fID, emscripten_webgl_get_current_context());
}
```

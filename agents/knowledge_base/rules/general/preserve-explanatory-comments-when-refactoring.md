---
name: skia/preserve-explanatory-comments-when-refactoring
description: When refactoring, extracting, or relocating existing methods or code blocks, preserve explanatory comments documenting non-obvious invariants, external contracts, or side effects
rubric: readability
severity: should_fix
---

**Rule.** When refactoring, extracting, or relocating existing methods,
functions, or implementation blocks, preserve existing explanatory comments that
document non-obvious logic, external dependencies, cross-language contracts, or
context side effects. Do not silently strip or discard rationale comments during
code reorganization unless the underlying invariant or contract has been
eliminated.

**Why.** Explanatory comments that describe interactions across subsystem
boundaries, cross-language bindings, hardware or driver quirks, or context side
effects provide crucial context for why the code is written a certain way.
Silently dropping them during refactorings increases cognitive overhead for
future maintainers and risks accidental regressions when non-obvious constraints
are forgotten.

## Examples

Bad:

```cpp
// Existing comments dropped during method extraction/relocation:
std::unique_ptr<GrExternalTexture> generateExternalTexture(GrMipMapped mipmapped) override {
    GrGLTextureInfo glInfo;
    glInfo.fID     = fCallback.call<uint32_t>("makeTexture");
    glInfo.fFormat = GR_GL_RGBA8;
    glInfo.fTarget = GR_GL_TEXTURE_2D;

    GrBackendTexture backendTexture(fInfo.width(), fInfo.height(), mipmapped, glInfo);
    return std::make_unique<ExternalWebGLTexture>(backendTexture, glInfo.fID, emscripten_webgl_get_current_context());
}
```

Good:

```cpp
// Explanatory comments preserved when relocating or adapting the method:
std::unique_ptr<GrExternalTexture> generateExternalTexture(GrRecordingContext* ctx,
                                                           GrMipMapped mipmapped) override {
    GrGLTextureInfo glInfo;

    // This callback is defined in webgl.js
    glInfo.fID     = fCallback.call<uint32_t>("makeTexture");

    // The format and target should match how we make the texture on the JS side.
    // See the implementation of the makeTexture function.
    glInfo.fFormat = GR_GL_RGBA8;
    glInfo.fTarget = GR_GL_TEXTURE_2D;

    GrBackendTexture backendTexture(fInfo.width(), fInfo.height(), mipmapped, glInfo);

    // In order to bind the image source to the texture, makeTexture has changed which
    // texture is "in focus" for the WebGL context.
    GrAsDirectContext(ctx)->resetContext(kTextureBinding_GrGLBackendState);
    return std::make_unique<ExternalWebGLTexture>(
        backendTexture, glInfo.fID, emscripten_webgl_get_current_context());
}
```

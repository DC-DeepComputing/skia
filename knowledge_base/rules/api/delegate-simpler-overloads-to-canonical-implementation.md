---
name: skia/api/delegate-simpler-overloads-to-canonical-implementation
description: When implementing function overloads or specialized factory wrappers, delegate to the fully parameterized canonical helper instead of duplicating setup boilerplate
rubric: design
severity: should_fix
---

**Rule.** When implementing overloaded functions, factory methods, or
specialized convenience wrappers where simpler variants query defaults or
provide specific configurations, delegate directly to the fully parameterized
canonical helper rather than duplicating initialization, table fetching,
resource creation, or validation logic. Keep common setup and validation
centralized in the canonical implementation.

**Why.** Duplicating setup, table or resource queries, memory allocation, and
validation across overloads or specialized wrappers increases maintenance
overhead, causes code bloat, and risks subtle divergence when initialization
logic or bug fixes are updated in only one path. Centralizing creation in the
canonical helper ensures uniform initialization.

## Examples

Bad:

```cpp
// Variant 1: Overload duplicating setup instead of delegating to parameterized overload
sk_sp<SkSurface> MakeOnScreenGLSurface(sk_sp<GrDirectContext> dContext, int width, int height,
                                       sk_sp<SkColorSpace> colorSpace) {
    GrGLint sampleCnt, stencil;
    emscripten_glGetIntegerv(GL_SAMPLES, &sampleCnt);
    emscripten_glGetIntegerv(GL_STENCIL_BITS, &stencil);
    emscripten_glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GrGLFramebufferInfo fbInfo{0, GL_RGBA8};
    GrBackendRenderTarget target(width, height, sampleCnt, stencil, fbInfo);
    return SkSurfaces::WrapBackendRenderTarget(dContext.get(), target, ...);
}

// Variant 2: Specialized factory wrapper duplicating resource fetching and validation
SkOTUtils::LocalizedStrings_NameTable*
SkOTUtils::LocalizedStrings_NameTable::CreateForFamilyNames(const SkTypeface& typeface) {
    static const SkFontTableTag nameTag = SkSetFourByteTag('n','a','m','e');
    size_t nameTableSize = typeface.getTableSize(nameTag);
    if (0 == nameTableSize) { return nullptr; }
    std::unique_ptr<uint8_t[]> nameTableData(new uint8_t[nameTableSize]);
    if (typeface.getTableData(nameTag, 0, nameTableSize, nameTableData.get()) != nameTableSize) {
        return nullptr;
    }
    return new SkOTUtils::LocalizedStrings_NameTable((SkOTTableName*)nameTableData.release(),
        familyNameTypes, SK_ARRAY_COUNT(familyNameTypes));
}
```

Good:

```cpp
// Variant 1: Query defaults and delegate to the fully parameterized overload
sk_sp<SkSurface> MakeOnScreenGLSurface(sk_sp<GrDirectContext> dContext, int width, int height,
                                       sk_sp<SkColorSpace> colorSpace) {
    GrGLint sampleCnt, stencil;
    emscripten_glGetIntegerv(GL_SAMPLES, &sampleCnt);
    emscripten_glGetIntegerv(GL_STENCIL_BITS, &stencil);
    return MakeOnScreenGLSurface(dContext, width, height, colorSpace, sampleCnt, stencil);
}

// Variant 2: Delegate specialized factory wrapper directly to the canonical helper
SkOTUtils::LocalizedStrings_NameTable*
SkOTUtils::LocalizedStrings_NameTable::CreateForFamilyNames(const SkTypeface& typeface) {
    return Create(typeface, familyNameTypes, SK_ARRAY_COUNT(familyNameTypes));
}
```

---
name: skia/gpu/validate-inputs-at-point-of-use-in-gpu-callees
description: When querying GPU backend capabilities or resource handlers, validate format and object validity at point-of-use in callees rather than caller wrappers
rubric: design
severity: should_fix
---

**Rule.** Validate object and format validity (such as format.isValid()) at the
point of use in low-level callee methods—such as GPU backend Caps
implementations and resource handlers—rather than placing defensive checks in
high-level callers or proxy wrappers. Ensure callee query methods return safe
fallback values (such as 0 or false) for invalid inputs instead of relying on
debug-only assertions like SkASSERT(format.isValid()).

**Why.** Defensive checks in high-level wrappers or proxy methods leave other
call paths vulnerable if low-level callees rely solely on debug assertions.
Validating inputs at the point of use in the callee protects all existing and
future callers against crashes in release builds.

## Examples

Bad:

```cpp
// High-level caller
GrBackendFormat format = fCaps->getDefaultBackendFormat(colorType, GrRenderable::kYes);
if (!format.isValid()) {
    return 0;
}
return fCaps->maxRenderTargetSampleCount(format);

// Low-level callee
int GrD3DCaps::maxRenderTargetSampleCount(const GrBackendFormat& format) const {
    SkASSERT(format.isValid());
    DXGI_FORMAT dxgiFormat = GrBackendFormats::AsDxgiFormat(format);
    return this->maxRenderTargetSampleCount(dxgiFormat);
}
```

Good:

```cpp
// High-level caller
GrBackendFormat format = fCaps->getDefaultBackendFormat(colorType, GrRenderable::kYes);
return fCaps->maxRenderTargetSampleCount(format);

// Low-level callee
int GrD3DCaps::maxRenderTargetSampleCount(const GrBackendFormat& format) const {
    if (!format.isValid()) {
        return 0;
    }
    DXGI_FORMAT dxgiFormat = GrBackendFormats::AsDxgiFormat(format);
    return this->maxRenderTargetSampleCount(dxgiFormat);
}
```

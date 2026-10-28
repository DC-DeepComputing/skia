---
name: skia/use-sk-abort-for-unimplemented-stubs
description: When stubbing out unimplemented methods, virtual overrides, or unsupported paths, terminate with SK_ABORT("Not implemented.") rather than returning dummy values, SkUNREACHABLE, or silent fallbacks
rubric: correctness
severity: should_fix
---

**Rule.** When providing stubs or default implementations for unimplemented
methods, virtual overrides, or unsupported fallback paths, terminate execution
using SK_ABORT("Not implemented."). Do not silently return dummy or
default-constructed values (such as empty containers or nullptr) that mask
missing functionality, do not provide silent fallbacks that discard
caller-requested parameters, and do not use bare SkUNREACHABLE;, which is
reserved strictly for structurally unreachable paths. Prefer the standard "Not
implemented." string literal over custom messages to enable compiler string
pooling across compilation units.

**Why.** Returning dummy fallback values or silently ignoring parameters masks
incomplete implementations, leading callers to proceed under false assumptions
or causing subtle correctness bugs. Failing loudly with SK_ABORT provides fatal
diagnostic logging at runtime, whereas SkUNREACHABLE expands to an untraced trap
instruction. Reusing the canonical string literal "Not implemented."
standardizes error messaging and enables string pooling across compilation
units.

## Examples

Bad:

```cpp
// Anti-pattern 1: returning dummy/empty containers from unimplemented stub
std::vector<Segment> Contours::segments() const {
    return std::vector<Segment>{};
}

// Anti-pattern 2: bare SkUNREACHABLE on reachable stub
bool Texture::uploadDataOnHost(const UploadSource& source, const SkIRect& dstRect) {
    SkUNREACHABLE;
}

// Anti-pattern 3: silent fallback that discards caller-requested proxying
std::unique_ptr<SkScalerContext> SkTypeface::onCreateScalerContextAsProxyTypeface(
        const SkScalerContextEffects& effects,
        const SkDescriptor* desc,
        SkTypeface* typeface) const {
    return this->onCreateScalerContext(effects, desc);
}

// Anti-pattern 4: ad-hoc custom abort message instead of standard string
void SkTypeface::onGetFamilyName(SkString* familyName) const {
    SK_ABORT("We should not use this call");
}
```

Good:

```cpp
// Good 1: fail fast with canonical literal on unimplemented stub
std::vector<Segment> Contours::segments() const {
    SK_ABORT("Not implemented.");
}

// Good 2: explicit SK_ABORT on unsupported or unimplemented override
bool Texture::uploadDataOnHost(const UploadSource& source, const SkIRect& dstRect) {
    SK_ABORT("Not implemented.");
}

// Good 3: fail loudly when requested capability is not supported
std::unique_ptr<SkScalerContext> SkTypeface::onCreateScalerContextAsProxyTypeface(
        const SkScalerContextEffects&,
        const SkDescriptor*,
        SkTypeface*) const {
    SK_ABORT("Not implemented.");
}

// Good 4: reuse canonical "Not implemented." literal for string pooling
void SkTypeface::onGetFamilyName(SkString* familyName) const {
    SK_ABORT("Not implemented.");
}
```

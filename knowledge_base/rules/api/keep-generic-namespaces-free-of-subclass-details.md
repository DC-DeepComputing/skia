---
name: skia/api/keep-generic-namespaces-free-of-subclass-details
description: When adding backend-, platform-, or format-specific functions or accessors, keep core classes and generic headers portable, relocating them to modular headers under plural namespaces
rubric: design
severity: should_fix
---

**Rule.** Do not add format-, subclass-, backend-, or platform-specific methods,
toggles, or implementation queries directly to core classes (such as SkTypeface,
SkImage, or SkSurface) or generic base headers and shared namespaces (such as
SkCodecs). Instead, place backend- or platform-specific entry points in modular
backend/port headers (e.g., under include/ports/ or include/gpu/) within
established plural namespaces (e.g., SkTypefaces, SkImages, SkSurfaces), in
dedicated subclass headers (such as SkAndroidCodec), or confine internal helpers
to src/.

**Why.** Core classes and generic base headers define portable, platform-neutral
public abstractions across all Skia embedders and target platforms. Attaching
backend- or platform-specific methods (even private or experimental accessors)
or toggles breaks encapsulation, pollutes the public interface for unaffected
platforms, and couples shared interfaces to concrete implementations instead of
maintaining Skia's modular port architecture.

## Examples

Bad:

```cpp
// include/codec/SkCodec.h
namespace SkCodecs {
// Bad: Platform- and format-specific toggle in generic decoder namespace
SK_API bool AvifEnabled();
}

// include/core/SkTypeface.h
class SkTypeface : public SkRefCnt {
public:
    // Bad: Backend-specific accessor added directly to portable core class
    void* internal_private_getIDWriteFontFace() const { return this->onGetDWriteFontFace(); }
};
```

Good:

```cpp
// include/codec/SkAndroidCodec.h
class SK_API SkAndroidCodec : SkNoncopyable {
public:
    // Good: Platform-specific query hosted on dedicated subclass header
    static bool AvifEnabled();
};

// include/ports/SkTypeface_win.h
namespace SkTypefaces::DirectWrite {
// Good: Backend-specific function declared in modular port header under plural namespace
IDWriteFontFace* GetIDWriteFontFace(const SkTypeface*);
}
```

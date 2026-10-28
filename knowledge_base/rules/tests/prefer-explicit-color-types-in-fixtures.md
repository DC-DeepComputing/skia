---
name: skia/tests/prefer-explicit-color-types-in-fixtures
description: When configuring test fixtures, harnesses, or tooling utilities, specify explicit color types like kRGBA_8888_SkColorType rather than platform-dependent kN32_SkColorType
rubric: readability
severity: should_fix
---

**Rule.** When configuring color types in test fixtures, test runners, mock
backends, or tooling utilities, specify explicit, fixed color types (such as
kRGBA_8888_SkColorType) instead of platform-dependent kN32_SkColorType. Avoid
convenience helpers like SkImageInfo::MakeN32Premul() or
SkSurface::MakeRasterN32Premul() when allocating bitmaps, creating surfaces, or
setting up test harnesses, unless testing platform-specific N32 behavior
explicitly.

**Why.** kN32_SkColorType is a platform-dependent alias that resolves to
kRGBA_8888_SkColorType or kBGRA_8888_SkColorType depending on host platform
architecture and byte order. Using it in tests and tools obscures whether pixels
are BGRA or RGBA, causing serialization ambiguities, platform-dependent blitter
selection, and CI test flakiness.

## Examples

Bad:

```cpp
// In test fixtures or runners:
class TestRunner {
    static constexpr SkColorType kColorType = kN32_SkColorType;
    skif::Backend fBackend = skif::MakeRasterBackend({}, kColorType);
};

// In tooling or pixel allocation:
bm.allocPixels(SkImageInfo::MakeN32Premul(10, 10));
```

Good:

```cpp
// In test fixtures or runners:
class TestRunner {
    static constexpr SkColorType kColorType = kRGBA_8888_SkColorType;
    skif::Backend fBackend = skif::MakeRasterBackend({}, kColorType);
};

// In tooling or pixel allocation:
bm.allocPixels(SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
```

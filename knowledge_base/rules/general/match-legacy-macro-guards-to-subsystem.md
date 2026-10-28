---
name: skia/match-legacy-macro-guards-to-subsystem
description: When guarding legacy APIs, factories, or fallbacks with SK_DISABLE_LEGACY_* or SK_SUPPORT_LEGACY_* directives, ensure the macro identifier matches the target subsystem
rubric: correctness
severity: should_fix
---

**Rule.** When guarding legacy APIs, factory overloads, or fallback paths with
conditional preprocessor directives (such as #if !defined(SK_DISABLE_LEGACY_...)
or #if defined(SK_SUPPORT_LEGACY_...)), verify that the macro identifier matches
the subsystem and component being guarded rather than an unrelated subsystem. Do
not copy macro guards from other subsystems or features without updating the
subsystem token.

**Why.** Preprocessor directives checking undefined macros silently evaluate to
false in #if defined(...) or true in #if !defined(...). Checking an unrelated
subsystem's macro (such as XPS in SVG code) prevents embedders and build
configurations from selectively disabling deprecated code or optional
dependencies, causing compilation or link failures.

## Examples

Bad:

```cpp
// In src/svg/SkSVGCanvas.cpp:
#if !defined(SK_DISABLE_LEGACY_XPS_FACTORIES)
    if (!opts.pngEncoder) {
        opts.pngEncoder = [](SkWStream* dst, const SkPixmap& src) {
            return SkPngEncoder::Encode(dst, src, {});
        };
    }
#endif
```

Good:

```cpp
// In src/svg/SkSVGCanvas.cpp:
#if !defined(SK_DISABLE_LEGACY_SVG_FACTORIES)
    if (!opts.pngEncoder) {
        opts.pngEncoder = [](SkWStream* dst, const SkPixmap& src) {
            return SkPngEncoder::Encode(dst, src, {});
        };
    }
#endif
```

---
name: skia/document-deprecated-dependency-fallbacks
description: When introducing temporary platform hooks, fallback logic, or coupled/monolithic build targets instead of modular injection or central libraries, document decoupling with a TODO
rubric: readability
severity: should_fix
---

**Rule.** When retaining temporary fallback logic in source code, introducing
platform-specific property checks in core components, defining build targets
that bundle optional dependencies rather than requiring caller injection, or
compiling shared sources directly into concrete backend targets instead of a
central common library, document with an attributed comment or TODO that this
arrangement is transitional. Clarify how the component or build target should be
decoupled (such as moving shared sources to a central cc_library so clients can
link non-conflicting targets, or having callers inject dependencies).

**Why.** Embedding platform-specific configuration checks or hardcoding optional
backend dependencies directly in core Skia and modules creates architectural
coupling and technical debt. Similarly, compiling common sources directly into
concrete backend targets causes symbol conflicts when embedding clients
configure multiple backends. Documenting transitional arrangements and coupled
build targets with an attributed TODO clarifies that the internal resolution or
monolithic target is not endorsed design and preserves the intended migration
path.

## Examples

Bad:

```cpp
// Source fallback:
#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
sk_sp<SkUnicode> get_unicode() { ... }

// Platform-specific property check in core registration:
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    if (property_get_int32("media.avif.crabbyavif", 0) != 0) {
        decoders->push_back(SkAvifDecoder::CrabbyAvif::Decoder());
    }
#endif

# Build target bundling shared sources with concrete backend:
skia_cc_library(
    name = "skunicode_icu",
    srcs = [
        "//modules/skunicode/src:icu_bidi_srcs",
        "//modules/skunicode/src:icu_srcs",
        "//modules/skunicode/src:srcs",
    ],
)
```

Good:

```cpp
// Source fallback documented with client-injection requirement:
// TODO(b/123): SVG clients must explicitly create and pass SkUnicode/SkShaper via builder APIs.
if (auto shaper = SkShapers::HB::ShaperDrivenWrapper(get_unicode(), fallback)) { ... }

// Platform-specific hook documented with framework decoupling TODO:
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    // TODO(b/123): Codec registration for Android framework belongs outside Skia; move this logic there.
    if (property_get_int32("media.avif.crabbyavif", 0) != 0) {
        decoders->push_back(SkAvifDecoder::CrabbyAvif::Decoder());
    }
#endif

# Build target documented with modularization TODO:
# TODO(kjlubick,bungeman): Break this up so clients can have non-conflicting
# skunicode targets (e.g. move shared srcs to a central cc_library).
skia_cc_library(
    name = "skunicode_icu",
    srcs = [
        "//modules/skunicode/src:icu_bidi_srcs",
        "//modules/skunicode/src:icu_srcs",
        "//modules/skunicode/src:srcs",
    ],
)
```

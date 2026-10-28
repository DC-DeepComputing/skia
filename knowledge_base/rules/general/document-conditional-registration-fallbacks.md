---
name: skia/document-conditional-registration-fallbacks
description: When conditionally registering components, codecs, or backends under runtime or platform checks without an else fallback, document why no alternative is registered or how the false condition is handled
rubric: readability
severity: nit
---

**Rule.** When conditionally registering or enabling a component, codec, or
backend behind a runtime flag or platform check without an else fallback branch,
document why no alternative is registered or how the format or operation is
otherwise handled when the condition evaluates to false. Explicitly clarify in
comments whether an alternative decoder or handler is provided elsewhere by the
platform or if the functionality is intentionally unavailable.

**Why.** Omitting an else branch during conditional component or codec
registration leaves it ambiguous whether an alternative fallback was
accidentally overlooked or is intentionally handled elsewhere by the host
platform. Documenting the fallback behavior and platform constraints preserves
architectural intent and prevents future maintainers from introducing redundant
or conflicting fallbacks.

## Examples

Bad:

```cpp
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    if (property_get_int32("media.avif.crabbyavif", 0) != 0) {
        decoders->push_back(SkAvifDecoder::CrabbyAvif::Decoder());
    }
#else
    decoders->push_back(SkAvifDecoder::Decoder());
#endif
```

Good:

```cpp
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    // Register CrabbyAvif based SkAvifDecoder on the Android framework
    // if it is allowed. Otherwise Android framework will use
    // SkHeifDecoder for decoding AVIF.
    if (property_get_int32("media.avif.crabbyavif", 0) != 0) {
        decoders->push_back(SkAvifDecoder::CrabbyAvif::Decoder());
    }
#else
    decoders->push_back(SkAvifDecoder::LibAvif::Decoder());
#endif
```

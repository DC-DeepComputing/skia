---
name: skia/codec/control-decoder-availability-via-registration
description: When gating decoder availability behind feature flags or platform properties, conditionally register decoders via SkCodecs::Register rather than adding bespoke query methods
rubric: design
severity: should_fix
---

**Rule.** Do not add bespoke runtime flag or property query methods (such as
AvifEnabled()) on codec wrapper classes to gate decoder usage. Instead, control
decoder availability by conditionally registering the decoder via
SkCodecs::Register or within the default decoder list, allowing call sites to
uniformly rely on SkCodecs::HasDecoder() or standard codec factory dispatch.

**Why.** Skia decoders are modular and decoupled through SkCodecs registration.
Exposing bespoke query methods on codec classes to inspect platform flags or
system properties leaks platform-specific switch logic into codec APIs and
causes inconsistent decoder availability checks across call sites.

## Examples

Bad:

```cpp
// In SkAndroidCodec.h / .cpp:
static bool AvifEnabled() {
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    return property_get_int32("media.avif.crabbyavif", 0) != 0;
#else
    return true;
#endif
}

// In SkAndroidCodec::MakeFromCodec:
if (SkCodecs::HasDecoder("avif") && SkAndroidCodec::AvifEnabled()) { ... }
```

Good:

```cpp
// In SkCodec.cpp (or client initialization registering decoders):
#if defined(SK_CODEC_DECODES_AVIF)
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    if (property_get_int32("media.avif.crabbyavif", 0) != 0) {
        decoders->push_back(SkAvifDecoder::CrabbyAvif::Decoder());
    }
#else
    decoders->push_back(SkAvifDecoder::LibAvif::Decoder());
#endif
#endif

// In SkAndroidCodec::MakeFromCodec:
if (SkCodecs::HasDecoder("avif")) { ... }
```

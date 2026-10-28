---
name: skia/codec/gate-codec-enablement-at-registration
description: When implementing codec format detection (Is<Format>), purely inspect stream signatures and gate conditional codec enablement at registration (SkCodecs::Register) rather than inside sniffing
rubric: design
severity: should_fix
---

**Rule.** Do not gate codec format detection (such as Is<Format> or
IsFormatCallback) on feature flags, system properties, or higher-level framework
state (such as SkAndroidCodec). Format sniffing must strictly validate the
stream header or magic byte signature; handle conditional codec enablement,
selection, or backend toggling at the decoder registration or dispatch layer
(such as SkCodecs::Register).

**Why.** Format sniffing callbacks must be pure stream validators. Gating format
detection on external feature flags incorrectly misreports valid streams as
unrecognized formats and introduces inverted dependency layering where low-level
codecs depend on higher-level framework wrappers. Gating at registration ensures
clean separation of concerns and lets the dispatcher fallback cleanly across
codecs.

## Examples

Bad:

```cpp
// In src/codec/SkAvifCodec.cpp
bool SkAvifCodec::IsAvif(const void* buffer, size_t bytesRead) {
    if (!SkAndroidCodec::AvifEnabled()) {
        return false;
    }
    avifROData avifData = {static_cast<const uint8_t*>(buffer), bytesRead};
    return avifPeekCompatibleFileType(&avifData) == AVIF_TRUE;
}
```

Good:

```cpp
// In src/codec/SkAvifCodec.cpp (pure format inspection)
bool SkAvifCodec::IsAvif(const void* buffer, size_t bytesRead) {
    avifROData avifData = {static_cast<const uint8_t*>(buffer), bytesRead};
    return avifPeekCompatibleFileType(&avifData) == AVIF_TRUE;
}

// In decoder registration (e.g. src/codec/SkCodec.cpp or client setup)
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
if (property_get_int32("media.avif.crabbyavif", 0) != 0) {
    decoders->push_back(SkAvifDecoder::CrabbyAvif::Decoder());
}
#endif
```

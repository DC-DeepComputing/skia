---
name: skia/codec/delegate-container-embedded-decoding
description: When implementing container image decoders (such as ICO), delegate embedded image payload decoding to dedicated image codecs rather than writing custom entry decoders
rubric: design
severity: should_fix
---

**Rule.** In container image decoders (such as ICO or CUR), delegate embedded
image stream decoding to existing dedicated format codecs (such as
SkPngRustCodec, SkBmpRustCodec, or SkCodec::MakeFromStream) rather than writing
custom or ad-hoc entry decoders. Query or sniff the embedded payload format and
instantiate the dedicated codec for each sub-image entry.

**Why.** Container image formats embed standard image formats like PNG and BMP.
Implementing ad-hoc entry decoders duplicates complex decoding logic and
routinely misses crucial features provided by dedicated codecs, such as embedded
ICC and cICP profiles, gAMA and cHRM metadata, and EXIF orientation handling.

## Examples

Bad:

```cpp
// Writing an ad-hoc entry decoder duplicates logic and misses color profile/EXIF support
class SkIcoEntryCodec : public SkCodec { ... };
std::unique_ptr<SkCodec> codec = SkIcoEntryCodec::Make(
        std::move(entryStream), entry.format, size, &entryResult);
```

Good:

```cpp
// Delegate embedded payload decoding to dedicated codecs to preserve full feature support
if (entry.format == EmbeddedFormat::kPng) {
    codec = SkPngRustCodec::MakeFromStream(std::move(entryStream), &entryResult);
} else {
    codec = SkBmpRustCodec::MakeFromStream(std::move(entryStream), &entryResult,
                                           SkBmpRustCodec::StreamType::kICO);
}
```

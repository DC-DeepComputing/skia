---
name: skia/tests/do-not-parameterize-invalid-input-failure-stages
description: When testing robustness against invalid or corrupted inputs in decoders or parsers, verify crash freedom without parameterizing creation versus decode failure stages
rubric: design
severity: should_fix
---

**Rule.** When testing parser or decoder robustness against invalid, truncated,
or corrupted inputs, do not parameterize test helpers with flags expecting
failures at specific stages (such as creation-time versus decode-time). Verify
crash freedom by allowing either stage to fail safely, or author separate
dedicated test functions if validating a specific failure phase is strictly
required.

**Why.** Asserting whether malformed or corrupted inputs fail at creation time
versus decode time makes tests brittle across different decoder backends and
parser refactorings that change when validation occurs. In invalid-input
robustness tests, the essential invariant is surviving corrupted data without
crashing or hanging.

## Examples

Bad:

```cpp
// Anti-pattern: parameterizing expected failure stage in invalid-input robustness tests
auto test = [&r](const char* description, const char* file, bool allowDecode) {
    sk_sp<SkData> data = GetResourceAsData(file);
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

    if (allowDecode) {
        if (codec) {
            auto [image, decodeResult] = codec->getImage();
        }
    } else {
        REPORTER_ASSERT(r, !codec, "Decoder should fail during creation on: %s", description);
    }
};
```

Good:

```cpp
// Verify crash freedom without enforcing whether creation or decode rejects invalid data
auto test = [&r](const char* description, const char* file) {
    sk_sp<SkData> data = GetResourceAsData(file);
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

    if (codec) {
        auto [image, decodeResult] = codec->getImage();
        (void)image;
        (void)decodeResult;
    }
};
```

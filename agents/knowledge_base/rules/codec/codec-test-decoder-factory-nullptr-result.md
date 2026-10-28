---
name: skia/codec/codec-test-decoder-factory-nullptr-result
description: When implementing or testing image decoder factories accepting optional SkCodec::Result* out-parameters, bind null pointers to local fallback storage and test that nullptr decodes without crashing
rubric: readability
severity: should_fix
---

**Rule.** When implementing or authoring unit tests for image decoders (such as
Sk<Format>Decoder::Decode or SkCodec::MakeFromStream), handle optional
SkCodec::Result* parameters at function entry by redirecting null pointers to a
local fallback variable (e.g., Result resultStorage; if (!result) { result =
&resultStorage; }) before assigning to *result. In test suites, include a
dedicated unit test verifying that passing nullptr for the optional Result*
parameter succeeds without crashing.

**Why.** SkCodec::Result* out-parameters are optional and default to nullptr in
public SkCodec and format decoder APIs. Dereferencing *result unconditionally
causes a null pointer dereference crash when callers omit the argument. Binding
null pointers to a local fallback variable at entry avoids scattering defensive
null checks across every error and exit path, while dedicated unit tests ensure
default-argument callers can safely instantiate decoders.

## Examples

Bad:

```cpp
// Bad: Decoder writes to *result without handling nullptr, or tests never pass nullptr:
std::unique_ptr<SkBmpRustCodec> SkBmpRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                               Result* result) {
    if (!stream) {
        *result = SkCodec::kInvalidParameters;
        return nullptr;
    }
    // ...
}

DEF_TEST(RustBmpCodec_Decode, r) {
    sk_sp<SkData> data = GetResourceAsData("images/randPixels.bmp");
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);
    REPORTER_ASSERT(r, codec && result == SkCodec::kSuccess);
}
```

Good:

```cpp
// Good: Decoder redirects null result to local storage, and unit test verifies nullptr:
std::unique_ptr<SkBmpRustCodec> SkBmpRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                               Result* result) {
    Result resultStorage;
    if (!result) {
        result = &resultStorage;
    }

    if (!stream) {
        *result = SkCodec::kInvalidParameters;
        return nullptr;
    }
    // ...
}

DEF_TEST(RustBmpCodec_nullptr_result, r) {
    sk_sp<SkData> data = GetResourceAsData("images/randPixels.bmp");
    REPORTER_ASSERT(r, data);

    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), nullptr);
    REPORTER_ASSERT(r, codec);
    REPORTER_ASSERT(r, codec->dimensions() == SkISize::Make(8, 8));
}
```

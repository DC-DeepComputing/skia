---
name: skia/codec/test-codec-icc-profile-metadata
description: When adding or modifying codec image metadata or embedded ICC profile extraction, add unit tests verifying getICCProfile(), SkColorSpace::Make, and successful image decoding
rubric: testing
severity: should_fix
---

**Rule.** When adding or modifying image metadata handling (such as embedded ICC
color profiles) in an image codec (SkCodec), add dedicated unit test coverage
using an asset containing that metadata. Verify that profile extraction
(codec->getICCProfile()) returns non-null, the extracted profile parses into a
valid color space (SkColorSpace::Make(*profile)), and image decoding
(codec->getImage()) succeeds with the profile applied.

**Why.** Generic decode tests typically use basic test assets that omit embedded
ICC profiles, completely skipping metadata extraction branches. A dedicated unit
test using a metadata-bearing asset ensures profile bytes and offsets are parsed
accurately without corruption or truncation, validates SkColorSpace integration,
and prevents silent regressions in color correction.

## Examples

Bad:

```cpp
// Bad: Extracting and binding an embedded ICC profile without dedicated unit test coverage.
rust::Vec<uint8_t> profileData = reader->icc_profile();
if (!profileData.empty()) {
    sk_sp<SkData> iccData = SkData::MakeWithCopy(profileData.data(), profileData.size());
    colorProfile = SkCodecs::ColorProfile::MakeICCProfile(std::move(iccData));
}
```

Good:

```cpp
// Good: Accompany metadata extraction with a unit test verifying getICCProfile and decoding.
DEF_TEST(Codec_ICCProfile_Extraction, r) {
    std::unique_ptr<SkCodec> codec =
            SkCodec::MakeFromStream(GetResourceAsStream("images/rgb24prof.bmp"));
    const skcms_ICCProfile* profile = codec->getICCProfile();
    REPORTER_ASSERT(r, profile, "Image with embedded ICC profile should report getICCProfile() != nullptr");

    sk_sp<SkColorSpace> colorSpace = SkColorSpace::Make(*profile);
    REPORTER_ASSERT(r, colorSpace, "Extracted ICC profile should yield a valid SkColorSpace");

    auto [image, result] = codec->getImage();
    REPORTER_ASSERT(r, image && result == SkCodec::kSuccess);
}
```

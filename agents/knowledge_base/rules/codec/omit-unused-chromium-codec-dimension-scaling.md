---
name: skia/codec/omit-unused-chromium-codec-dimension-scaling
description: When implementing image codecs or codec tests targeted for Chromium integration, omit unused dimension scaling overrides and tests when the client uses frame indexing
rubric: design
severity: should_fix
---

**Rule.** When implementing image codecs (especially multi-image container
formats like ICO) or codec unit tests targeted for Chromium integration, avoid
implementing or testing dimension scaling APIs (such as onGetScaledDimensions,
onDimensionsSupported, or arbitrary scaled decode via getPixels). Rely on base
SkCodec default non-scaling implementations instead when downstream clients
select multi-resolution assets exclusively via frame indexing.

**Why.** Chromium's image decoding pipeline (such as Blink) accesses
multi-resolution container images by querying frame counts and selecting
specific frame indices, rather than requesting scaled dimensions via getPixels.
Implementing unused scaling overrides and tests increases code complexity, test
execution time, and ongoing maintenance burden without providing client value.

## Examples

Bad:

```cpp
// Overriding optional dimension scaling methods in SkCodec subclass:
SkISize onGetScaledDimensions(float desiredScale) const override;
bool onDimensionsSupported(const SkISize&) override;

// Testing arbitrary dimension scaling on a container codec:
DEF_TEST(RustIcoCodec_decode_specific_size, r) {
    SkImageInfo info = codec->getInfo().makeDimensions(SkISize::Make(16, 16));
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    SkCodec::Result result = codec->getPixels(bitmap.pixmap());
}
```

Good:

```cpp
// Rely on base SkCodec default non-scaling implementations:
// virtual SkISize onGetScaledDimensions(float) const { return this->dimensions(); }
// virtual bool onDimensionsSupported(const SkISize&) { return false; }

// Test multi-resolution decoding via frame index as used by Chromium:
int frameCount = codec->getFrameCount();
for (int i = 0; i < frameCount; ++i) {
    SkCodec::FrameInfo frameInfo;
    REPORTER_ASSERT(r, codec->getFrameInfo(i, &frameInfo));
}
```

---
name: skia/decode-embedded-images-via-deferred-from-encoded-data
description: 'When decoding embedded image assets in font backends or subsystems, use SkImages::DeferredFromEncodedData with any_decoder instead of compile-time #ifdef branching and split targets'
rubric: design
severity: should_fix
---

**Rule.** When decoding embedded image assets (such as bitmap font glyphs or
embedded document resources), decode them using
SkImages::DeferredFromEncodedData or SkCodecs::DeferredImage rather than
hardcoding concrete decoders or using compile-time #ifdef guards. In build
configurations, depend generically on //src/codec:any_decoder instead of
duplicating library targets for each decoder backend.

**Why.** Hardcoding specific decoder backends or toggling them via compile-time
preprocessor macros and separate build targets (such as _libpng vs. _rustpng)
creates One-Definition-Rule (ODR) violations in monorepos and leads to build
target explosion. Using Skia's runtime codec registration architecture decouples
subsystems from specific decoders, allowing downstream embedders to register
their preferred format decoders via SkCodecs::Register without modifying
asset-consuming libraries.

## Examples

Bad:

```cpp
// In SkTypeface_fontations.cpp:
#if defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG)
#include "include/codec/SkPngDecoder.h"
#elif defined(SK_CODEC_DECODES_PNG_WITH_RUST)
#include "experimental/rust_png/decoder/SkPngRustDecoder.h"
#endif

// In BUILD.bazel:
skia_cc_library(name = "fontations_support_libpng", ...)
skia_cc_library(name = "fontations_support_rustpng", ...)
```

Good:

```cpp
// In SkTypeface_fontations.cpp:
sk_sp<SkImage> img = SkImages::DeferredFromEncodedData(
        SkData::MakeWithoutCopy(png_data.data(), png_data.size()));

// In BUILD.bazel:
skia_cc_library(
    name = "fontations_support",
    deps = [
        "//:core",
        "//src/codec:any_decoder",
    ],
)
```

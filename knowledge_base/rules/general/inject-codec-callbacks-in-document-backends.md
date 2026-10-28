---
name: skia/inject-codec-callbacks-in-document-backends
description: 'When implementing document or canvas export backends, inject image encoder and decoder callbacks via options structs rather than hardcoding or #ifdef-ing codec dependencies'
rubric: design
severity: should_fix
---

**Rule.** Do not hardcode image codec dependencies (such as SkPngEncoder or
SkPngRustEncoder) or use compile-time #ifdef guards to select codecs inside
document or canvas export backends. Instead, accept an options or metadata
struct containing encoder and decoder callback function pointers (such as
EncodePngCallback or jpegEncoder) so callers can inject their preferred codec
implementation.

**Why.** Hardcoding codec dependencies or toggling them via compile-time
preprocessor guards forces document and canvas backends to transitively depend
on concrete codec libraries (such as libpng). This causes link errors for
downstream clients that compile without those libraries. Passing callbacks via
options structs decouples backends and allows clients to supply custom codecs or
omit unused dependencies.

## Examples

Bad:

```cpp
// In src/xps/SkXPSDevice.cpp:
#ifdef SK_CODEC_ENCODES_PNG_WITH_RUST
#include "include/encode/SkPngRustEncoder.h"
#else
#include "include/encode/SkPngEncoder.h"
#endif
...
#ifdef SK_CODEC_ENCODES_PNG_WITH_RUST
    bool success = SkPngRustEncoder::Encode(&write, bitmap, {});
#else
    bool success = SkPngEncoder::Encode(&write, bitmap, {});
#endif
```

Good:

```cpp
// In include/docs/SkXPSDocument.h:
struct Options {
    float dpi = SK_ScalarDefaultRasterDPI;
    SkXPS::EncodePngCallback pngEncoder = nullptr;
};
SK_API sk_sp<SkDocument> MakeDocument(SkWStream* stream,
                                      IXpsOMObjectFactory* xpsFactory,
                                      SkXPS::Options opts);

// In src/xps/SkXPSDevice.cpp:
if (!fOpts.pngEncoder || !fOpts.pngEncoder(&write, bitmap)) {
    HRM(E_FAIL, "Unable to encode bitmap as png.");
}
```

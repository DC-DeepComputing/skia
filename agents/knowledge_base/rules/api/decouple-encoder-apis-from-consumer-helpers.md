---
name: skia/api/decouple-encoder-apis-from-consumer-helpers
description: When exposing or adapting encoder and decoder APIs for callback interfaces, avoid public wrappers or default-binding helper headers; have callers pass stateless lambdas
rubric: design
severity: should_fix
---

**Rule.** Do not add consumer-specific or document-format-specific adapter
functions (such as default-option callback wrappers) to general-purpose codec
and encoder headers in include/encode/ or include/codec/. Furthermore, avoid
creating public wrapper functions or dedicated helper headers merely to bind
default arguments to match a callback signature; have callers pass a stateless
lambda at the call site instead. Reserve dedicated consumer helper headers (such
as SkPDFJpegHelpers.h) for non-trivial adaptation requiring custom parameters or
metadata configuration.

**Why.** Core codec and encoder headers must remain decoupled from specific
document generators (like PDF, XPS, or SVG) and their callback signatures.
Adding public API wrappers or standalone helper headers merely to bind default
arguments introduces unnecessary API surface and maintenance overhead; stateless
lambdas convert implicitly to function pointers, keep the public API minimal,
and make configuration explicit at call sites.

## Examples

Bad:

```cpp
// Bad: Adding default-option callback wrappers to encoder headers:
// include/encode/SkPngRustEncoder.h
SK_API bool EncodeWithDefaultOptions(SkWStream* dst, const SkPixmap& src);

// Bad: Creating public wrapper functions or helper headers merely to bind defaults:
// include/encode/SkPngEncoder.h or include/docs/SkXPSPngHelpers.h
inline bool EncodeUsingDefaultOptions(SkWStream* dst, const SkPixmap& src) {
    return Encode(dst, src, {});
}
xpsOpts.pngEncoder = SkPngEncoder::EncodeUsingDefaultOptions;
```

Good:

```cpp
// Good: Callers pass a stateless lambda to bind default options at the call site:
xpsOpts.pngEncoder = [](SkWStream* dst, const SkPixmap& src) {
    return SkPngEncoder::Encode(dst, src, {});
};

// Good: Dedicated helper headers are reserved for non-trivial parameter adaptation:
// include/docs/SkPDFJpegHelpers.h
namespace SkPDF::JPEG {
inline bool Encode(SkWStream* dst, const SkPixmap& src, int quality) {
    SkJpegEncoder::Options jOpts;
    jOpts.fQuality = quality;
    return SkJpegEncoder::Encode(dst, src, jOpts);
}
}
```

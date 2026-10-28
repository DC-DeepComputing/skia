---
name: skia/tests/use-public-decoder-apis
description: When decoding images outside src/codec or in tests, use explicit Sk<Format>Decoder::Decode APIs rather than private codec classes or implicit SkCodec::MakeFromData
rubric: design
severity: blocking
---

**Rule.** When decoding image streams or byte buffers outside `src/codec/` (such
as in tests, GM tests, tools, or external modules), use public decoder APIs from
`include/codec/Sk<Format>Decoder.h` (such as `SkPngDecoder::Decode` or
`SkJpegDecoder::Decode`) rather than private codec classes from `src/codec/`
(such as `SkPngCodec::MakeFromStream`). Do not use `SkCodec::MakeFromData` or
`MakeFromStream` with implicit decoder resolution; call explicit format decoders
or pass an explicit decoder span (`SkSpan<const SkCodecs::Decoder>`), especially
when testing a specific codec backend (such as `SkPngRustDecoder::Decode`) or
running under Bazel builds.

**Why.** Internal codec classes under `src/codec/` do not export symbols with
`SK_API`, causing build and link failures outside `src/codec/`. Furthermore,
`SkCodec::MakeFromData` and `MakeFromStream` with implicit decoders rely on
legacy compile-time initialization disabled under Bazel
(`SK_DISABLE_LEGACY_INIT_DECODERS`), causing codec creation to fail, and rely on
global registration order which creates ambiguity when multiple backends (such
as libpng and Rust) are compiled in.

## Examples

Bad:

```cpp
// Bad: Referencing unexported internal codec classes outside src/codec/
#include "src/codec/SkPngCodec.h"
std::unique_ptr<SkCodec> codec =
        SkPngCodec::MakeFromStream(std::move(stream), &result, nullptr);

// Bad: Calling generic implicit dispatch (fails under Bazel SK_DISABLE_LEGACY_INIT_DECODERS)
auto codec = SkCodec::MakeFromData(wstream.detachAsData());
```

Good:

```cpp
// Good: Using public decoder APIs from include/codec/
#include "include/codec/SkPngDecoder.h"
std::unique_ptr<SkCodec> codec =
        SkPngDecoder::Decode(std::move(stream), &result);

// Good: Explicit format decoder avoiding ambiguity and implicit init
#include "include/codec/SkPngRustDecoder.h"
SkCodec::Result result;
auto rustCodec = SkPngRustDecoder::Decode(wstream.detachAsData(), &result);
```

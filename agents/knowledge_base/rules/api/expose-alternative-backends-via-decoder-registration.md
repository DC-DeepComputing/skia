---
name: skia/api/expose-alternative-backends-via-decoder-registration
description: 'When introducing alternative image codec backends, expose distinct SkCodecs::Decoder factories instead of conditionally swapping implementations with preprocessor #ifdef guards inside codec classes'
rubric: design
severity: should_fix
---

**Rule.** Do not use preprocessor `#define` or platform `#if defined(...)`
guards to conditionally swap out codec implementations within a codec class or
header. Instead, implement alternative codec backends as separate codecs and
expose them as distinct decoder factories (`SkCodecs::Decoder`, such as under
nested namespaces in `Sk<Format>Decoder` or dedicated decoder headers) so
clients can register their chosen implementation via `SkCodecs::Register`.

**Why.** Skia's codec architecture relies on explicit client-side registration
via `SkCodecs::Register(...)`. Conditionally swapping third-party backend
implementations inside a single codec class using compile-time preprocessor
macros breaks modular registration, prevents downstream clients from selecting
or overriding backends at runtime or configuration time, and leaks platform
build flags into portable codec headers.

## Examples

Bad:

```cpp
// In src/codec/SkAvifCodec.h:
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
namespace crabbyavif {
struct avifDecoder;
}  // namespace crabbyavif
using crabbyavif::avifDecoder;
#else
struct avifDecoder;
#endif  // defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
```

Good:

```cpp
// In src/codec/SkAvifCodec.h:
struct avifDecoder;

// In include/codec/SkAvifDecoder.h:
namespace SkAvifDecoder {
namespace LibAvif {
inline constexpr SkCodecs::Decoder Decoder() {
    return { "avif", IsAvif, Decode };
}
}  // namespace LibAvif

namespace CrabbyAvif {
inline constexpr SkCodecs::Decoder Decoder() {
    return { "avif", IsAvif, Decode };
}
}  // namespace CrabbyAvif
}  // namespace SkAvifDecoder
```

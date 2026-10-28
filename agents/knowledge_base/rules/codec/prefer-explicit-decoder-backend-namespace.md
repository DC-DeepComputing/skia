---
name: skia/codec/prefer-explicit-decoder-backend-namespace
description: When referencing or registering decoders offering multiple backend implementations under nested namespaces (such as SkAvifDecoder), explicitly qualify the backend namespace instead of default aliases
rubric: readability
severity: nit
---

**Rule.** When registering or referencing codec decoders that provide multiple
backend implementations under nested namespaces (such as SkAvifDecoder::LibAvif
vs. SkAvifDecoder::CrabbyAvif), explicitly qualify the concrete backend
namespace (e.g., SkAvifDecoder::LibAvif::Decoder()) rather than relying on
top-level backwards-compatibility default aliases (e.g.,
SkAvifDecoder::Decoder()). In conditional branches (#if / #else) selecting
between implementations, maintain symmetry by explicitly qualifying the backend
namespace on all branches.

**Why.** Top-level default aliases in multi-backend decoder namespaces exist
solely for backwards compatibility. Explicitly qualifying the backend
implementation documents which engine is being bound, prevents ambiguity, and
maintains symmetry across build- or platform-specific conditional branches.

## Examples

Bad:

```cpp
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    decoders->push_back(SkAvifDecoder::CrabbyAvif::Decoder());
#else
    // Anti-pattern: relying on the root namespace default alias
    decoders->push_back(SkAvifDecoder::Decoder());
#endif
```

Good:

```cpp
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    decoders->push_back(SkAvifDecoder::CrabbyAvif::Decoder());
#else
    // Explicitly qualify the chosen backend implementation
    decoders->push_back(SkAvifDecoder::LibAvif::Decoder());
#endif
```

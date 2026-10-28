---
name: skia/accurately-document-feature-support
description: When documenting feature support, external specifications, limitations, or simplifying assumptions, accurately describe implemented subsets, constraints, and backend applicability
rubric: readability
severity: should_fix
---

**Rule.** When documenting component capabilities, limitations, simplifying
assumptions, or deferred work in docs, headers, release notes, or comments,
ensure descriptions accurately reflect implementation status and backend
applicability; never claim support for unimplemented features. Visibly document
known limitations, unsupported options, intentional test fallbacks, or
simplifying assumptions that restrict constructs permitted by a specification.
When implementing an external standard or specification partially (such as
Unicode or media codecs), document which subsets or cases are supported and any
intentional limitations. Distinguish capabilities fundamentally unsupported by a
specification from functionality temporarily unimplemented.

**Why.** Overstating component support, omitting backend restrictions, or
leaving partial specification conformance undocumented misleads embedders and
maintainers about what Skia handles. Documenting supported subsets, intentional
limitations, backend scope, and simplifying assumptions prevents missing cases
from being misinterpreted as bugs and clarifies architectural boundaries without
inspecting review threads.

## Examples

Bad:

```cpp
<!-- README.md: claiming unimplemented features -->
- **Advanced**: Embedded JPEG/PNG, ICO containers, OS/2 bitmap variants

<!-- relnotes/cache_purge_time_limit.md: backend restriction omitted -->
Allow clients to pass in a maximum time duration when purging resources from caches.

// tests/CodecTest.cpp: unexplained fallback to legacy backend
DEF_TEST(PngRustHdrMetadataRoundTrip, r) {
    SkPngEncoder::Options options;
    ...
    auto codec = SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(wstream.detachAsData()), &result);
}

// modules/skunicode/src/SkUnicode.cpp: partial spec implementation without documenting supported subset
SkUnichar SkUnicode::getEmojiSequenceStart(const char** begin, const char* end) {
    // Incomplete implementation of Unicode TR51 emoji sequence without documenting what subset is supported
    ...
}

// src/sksl/codegen/SkSLWGSLCodeGenerator.cpp: claiming spec limitation instead of deferred work
// Compute programs don't have stage inputs or outputs, like vertex/fragment programs do.
```

Good:

```cpp
<!-- README.md: accurate support list -->
- **Advanced**: OS/2 bitmap variants (ICO and embedded JPEG/PNG not supported)

<!-- relnotes/cache_purge_time_limit.md: explicit backend restriction -->
The following new feature is only supported by the Graphite backend:
Allow clients to pass in a maximum time duration when purging resources from caches.

// tests/CodecTest.cpp: documented parity gap
DEF_TEST(PngRustHdrMetadataRoundTrip, r) {
    // SkPngRustEncoder does not support writing HDR metadata; using libpng encoder.
    SkPngEncoder::Options options;
    ...
    auto codec = SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(wstream.detachAsData()), &result);
}

// modules/skunicode/src/SkUnicode.cpp: documented supported subset and grammar limitations
// By definition any emoji_sequence starts from a codepoint that has UCHAR_EMOJI property.
// emoji_flag_sequence   := regional_indicator regional_indicator
// emoji_keycap_sequence := [0-9#*] \x{FE0F 20E3}
// These two cases require additional checks of the next codepoint(s).
SkUnichar SkUnicode::getEmojiSequenceStart(const char** begin, const char* end) {
    ...
}

// src/sksl/codegen/SkSLWGSLCodeGenerator.cpp: accurately distinguishing spec vs implementation status
// Compute programs don't have stage outputs, and stage inputs are not yet implemented.
```

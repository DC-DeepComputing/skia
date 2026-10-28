---
name: skia/document-concepts-over-skanysubclass
description: When implementing custom inline byte storage constrained by C++20 concepts instead of SkAnySubclass, document why base-class polymorphism is unsuitable in the header
rubric: readability
severity: nit
---

**Rule.** When implementing custom inline byte storage constrained by C++20
concepts instead of standard polymorphism utilities like SkAnySubclass, document
the design rationale in the header. Specifically explain why a common
polymorphic base class is unsuitable, such as requiring static factory methods,
avoiding per-element vtable pointer overhead for small fine-grained types, or
enforcing compile-time type transformations.

**Why.** SkAnySubclass is the standard Skia utility for storing polymorphic
backend-specific data inline without dynamic heap allocations. Bypassing it in
favor of raw byte buffers and concepts introduces unconventional indirection and
manual lifetime management that prompts maintenance questions unless the
architectural trade-offs are documented.

## Examples

Bad:

```cpp
// Custom inline storage for backend data without explanation.
class GlyphVector {
    using GlyphBytes = std::array<std::byte, sizeof(void*)>;
    using BackendDataBytes = std::array<std::byte, 88>;

    template <typename B>
    void initBackendData(auto&&... args);
    // ...
};
```

Good:

```cpp
/**
 * Concepts describing backend-specific (Ganesh vs. Graphite) data in GlyphVector.
 * We use concepts and inline byte arrays rather than SkAnySubclass or a base class
 * because BackendData requires static factory methods and per-glyph storage must
 * avoid vtable pointer overhead on fine-grained types.
 */
namespace GlyphVector_Concepts {
template <typename T>
concept BackendData = requires(T& t, SkPackedGlyphID id) {
    { t.makeGlyphFromID(id) } -> GlyphType;
};
}  // namespace GlyphVector_Concepts
```

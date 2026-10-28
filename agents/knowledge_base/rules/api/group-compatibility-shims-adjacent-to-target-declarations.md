---
name: skia/api/group-compatibility-shims-adjacent-to-target-declarations
description: When retaining backwards-compatibility forwarding functions or shims in outer scopes, place them adjacent to the target namespace and document where they route
rubric: readability
severity: should_fix
---

**Rule.** When refactoring APIs into nested namespaces or sibling submodules
while retaining backwards-compatibility forwarding functions, type aliases, or
shims at enclosing scope, place those shims immediately adjacent to the specific
namespace they delegate to and document their compatibility role and routing
target with a clarifying comment. Do not leave compatibility shims undocumented
or place them after unrelated sibling namespaces.

**Why.** Placing backwards-compatibility shims directly adjacent to the target
namespace and documenting their routing prevents confusion when identical
function or type declarations exist at both enclosing and nested scopes. Without
adjacent placement and explicit documentation, readers and API users may mistake
the shim for an active canonical API, assume it routes to an adjacent unrelated
sibling, or be unaware of the nested implementation.

## Examples

Bad:

```cpp
namespace LibAvif {
    inline constexpr SkCodecs::Decoder Decoder() { return { "avif", IsAvif, Decode }; }
}

namespace CrabbyAvif {
    inline constexpr SkCodecs::Decoder Decoder() { return { "avif", IsAvif, Decode }; }
}

// Bad: Undocumented and separated from LibAvif by an unrelated sibling namespace:
inline constexpr SkCodecs::Decoder Decoder() {
    return { "avif", LibAvif::IsAvif, LibAvif::Decode };
}
```

Good:

```cpp
namespace LibAvif {
    inline constexpr SkCodecs::Decoder Decoder() { return { "avif", IsAvif, Decode }; }
}

// Good: Documented compatibility shim placed immediately adjacent to LibAvif:
// This function is in the root SkAvifDecoder namespace. It simply routes
// everything through the LibAvif namespace and exists for backwards compatibility.
inline constexpr SkCodecs::Decoder Decoder() {
    return { "avif", LibAvif::IsAvif, LibAvif::Decode };
}

namespace CrabbyAvif {
    inline constexpr SkCodecs::Decoder Decoder() { return { "avif", IsAvif, Decode }; }
}
```

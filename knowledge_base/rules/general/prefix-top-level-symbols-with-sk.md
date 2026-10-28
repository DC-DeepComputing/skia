---
name: skia/prefix-top-level-symbols-with-sk
description: When declaring top-level symbols or namespaces in headers, prefix entities with Sk (or Gr in Ganesh) or enclose them within an Sk-prefixed namespace
rubric: readability
severity: should_fix
---

**Rule.** Do not introduce unprefixed types, enums, constants, functions, or
namespaces into the global namespace in header files. Prefix all top-level
symbols and namespaces with 'Sk' (or 'Gr' for Ganesh code), or enclose related
definitions within an Sk-prefixed namespace where nested types do not require
the prefix.

**Why.** Skia headers are consumed across external embedding projects and
internal subsystems. Declaring unprefixed, generic identifiers (such as enum
classes, helper functions, or constants) in the global namespace causes symbol
pollution, risks naming collisions with client or third-party code, and obscures
codebase ownership.

## Examples

Bad:

```cpp
// Bad: Unprefixed enum, constant, and free function placed in the global namespace in a header
// src/core/SkKnownRuntimeEffects.h
static constexpr int kSkiaBuiltInReservedCnt = 500;
enum class StableKey : uint32_t {
    k1DBlur,
    k2DBlur,
};
SkRuntimeEffect* GetKnownRuntimeEffect(StableKey);
```

Good:

```cpp
// Good: Scoped inside an Sk-prefixed namespace to avoid global namespace pollution
// src/core/SkKnownRuntimeEffects.h
namespace SkKnownRuntimeEffects {
static constexpr int kSkiaBuiltInReservedCnt = 500;
enum class StableKey : uint32_t {
    k1DBlur,
    k2DBlur,
};
SkRuntimeEffect* GetKnownRuntimeEffect(StableKey);
}  // namespace SkKnownRuntimeEffects
```

---
name: skia/avoid-wrapping-skenumbitmask
description: When defining or accepting bitmask flags, declare SK_MAKE_BITMASK_OPS, use SkEnumBitMask directly without wrapper structs, and omit redundant bare-enum overloads
rubric: readability
severity: should_fix
---

**Rule.** When defining an enum class for bitwise flags, declare
SK_MAKE_BITMASK_OPS(E) immediately following the definition to enable type-safe
bitwise operators, and use SkEnumBitMask<E> directly rather than creating custom
wrapper structs. Additionally, do not define redundant function or constructor
overloads taking a bare enum E when an overload taking SkEnumBitMask<E> is
present, as SkEnumBitMask<E> already provides an implicit converting constructor
from E.

**Why.** SkEnumBitMask<E> provides type-safe bitwise operators (|, &, ^, ~),
equality comparisons, and explicit boolean conversions when enabled via
SK_MAKE_BITMASK_OPS(E), making custom wrapper structs redundant boilerplate.
Furthermore, because SkEnumBitMask<E> has an implicit converting constructor
from E, accepting SkEnumBitMask<E> directly allows callers to pass either
individual enum values or bitwise combinations without dedicated overloads.

## Examples

Bad:

```cpp
// Bad: missing SK_MAKE_BITMASK_OPS on bitflag enum class
enum class RenderStateFlags : unsigned {
    kNone            = 0,
    kAppendVertices  = 1 << 0,
    kAppendInstances = 1 << 1,
};

// Bad: wrapping SkEnumBitMask in a boilerplate struct with redundant overloads
struct DstUsage {
    enum class Flags : uint8_t {
        kNone         = 0,
        kDependsOnDst = 1 << 0,
    };
    SkEnumBitMask<Flags> fFlags;
    constexpr DstUsage(Flags f) : fFlags(f) {}
    constexpr DstUsage(SkEnumBitMask<Flags> f) : fFlags(f) {}
};
```

Good:

```cpp
// Good: declare SK_MAKE_BITMASK_OPS to enable type-safe bitmask operators
enum class RenderStateFlags : unsigned {
    kNone            = 0,
    kAppendVertices  = 1 << 0,
    kAppendInstances = 1 << 1,
};
SK_MAKE_BITMASK_OPS(RenderStateFlags)

// Good: use SkEnumBitMask<T> directly; a single overload accepts both bare enum and mask
enum class DstUsage : uint8_t {
    kNone         = 0,
    kDependsOnDst = 1 << 0,
};
SK_MAKE_BITMASK_OPS(DstUsage)

void applyUsage(SkEnumBitMask<DstUsage> flags);
```

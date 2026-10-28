---
name: skia/prefer-value-members-over-unique-ptr
description: When declaring class or struct member variables, store owned objects directly by value instead of wrapping in std::unique_ptr unless dynamic allocation is required
rubric: safety
severity: should_fix
---

**Rule.** Avoid wrapping owned member objects in std::unique_ptr when they can
be held directly by value. Reserve std::unique_ptr for members that genuinely
require runtime polymorphism, deferred or lazy construction, or PIMPL
encapsulation of incomplete forward-declared types.

**Why.** Holding member objects directly by value eliminates unnecessary dynamic
heap allocation overhead, eliminates pointer indirection, and improves CPU cache
locality and memory density.

## Examples

Bad:

```cpp
struct TypefaceUse {
    SkTypefaceID typefaceId;
    int ttcIndex;
    std::unique_ptr<SkBitSet> glyphsUsed;
};

// Unnecessary heap allocation for an object with a known complete type
typefaceUse.glyphsUsed = std::make_unique<SkBitSet>(glyphCount);
```

Good:

```cpp
struct TypefaceUse {
    SkTypefaceID typefaceId;
    int ttcIndex;
    SkBitSet glyphsUsed;
};

// Directly initialized member object without heap indirection
TypefaceUse use{typefaceId, ttcIndex, SkBitSet(glyphCount)};
```

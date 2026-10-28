---
name: skia/name-float-bit-constants-with-bits
description: When declaring integer constants that store floating-point bit patterns, include Bits in the identifier (e.g., kFloatBits1) to distinguish them from float values
rubric: readability
severity: nit
---

**Rule.** When defining integer constants (such as uint32_t, uint64_t, or
uint16_t) that represent the raw IEEE-754 bit pattern of floating-point values,
include Bits in the identifier (e.g., kFloatBits1, kHalfBitsOne) rather than
naming them like floating-point constants (e.g., kFloat1). Distinguish raw bit
patterns from actual float scalar constants to prevent confusion about their
underlying type and arithmetic behavior.

**Why.** An integer constant named kFloat1 misleadingly implies a float scalar
type, masking that it holds a raw integer bit representation (uint32_t).
Explicitly incorporating Bits in the identifier clarifies the variable's type
and semantics, aligns with conversion helpers like SkFloat2Bits, and prevents
mistakes where bit representations are mistakenly used in scalar floating-point
expressions.

## Examples

Bad:

```cpp
// Bad: kFloat1 implies a float variable rather than an integer bit pattern
static constexpr uint32_t kFloat1 = 0x3f800000; // SkFloat2Bits isn't constexpr
```

Good:

```cpp
// Good: kFloatBits1 clearly identifies that the integer stores IEEE-754 bits
static constexpr uint32_t kFloatBits1 = 0x3f800000; // SkFloat2Bits isn't constexpr
SkASSERT(kFloatBits1 == SkFloat2Bits(1.f));
```

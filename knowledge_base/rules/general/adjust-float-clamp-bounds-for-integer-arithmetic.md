---
name: skia/adjust-float-clamp-bounds-for-integer-arithmetic
description: When clamping floating-point values before casting to integers followed by arithmetic offsets, adjust clamp bounds by the offset to prevent integer overflow or underflow
rubric: correctness
severity: should_fix
---

**Rule.** When clamping floating-point values before casting them to integers
that will undergo subsequent arithmetic (such as adding or subtracting an
offset), adjust the floating-point clamp bounds by that arithmetic offset. Do
not clamp to raw integer boundary limits if subsequent integer operations would
push boundary values past the type's minimum or maximum limits.

**Why.** Clamping floats directly to raw integer type limits prevents
float-to-int conversion undefined behavior, but subsequent arithmetic on the
converted integer can immediately underflow or overflow signed integer bounds.
Incorporating the arithmetic offset directly into the clamp bounds guarantees
that the resulting integer remains strictly within valid bounds after all
operations.

## Examples

Bad:

```cpp
static constexpr float kInt32Min = static_cast<float>(std::numeric_limits<int32_t>::min());
static constexpr float kInt32Max = static_cast<float>(std::numeric_limits<int32_t>::max());
fx = max(min(fx, kInt32Max), kInt32Min);
I32 qx = cast<I32>(fx) - 32768; // Underflows int32_t when fx == kInt32Min
```

Good:

```cpp
// Offset the clamp bounds to guarantee post-subtraction values fit in int32_t.
static constexpr float kInt32Min = static_cast<float>(std::numeric_limits<int32_t>::min()) + 32768.0f;
static constexpr float kInt32Max = static_cast<float>(std::numeric_limits<int32_t>::max());
fx = max(min(fx, kInt32Max), kInt32Min);
I32 qx = cast<I32>(fx) - 32768; // Safe from underflow
```

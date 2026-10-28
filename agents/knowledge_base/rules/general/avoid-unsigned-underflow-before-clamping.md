---
name: skia/avoid-unsigned-underflow-before-clamping
description: When subtracting offsets or computing values that can drop below zero before clamping, use signed types to prevent unsigned underflow
rubric: correctness
severity: should_fix
---

**Rule.** Do not use unsigned integer types when subtracting an offset or
computing intermediate values that can drop below zero prior to clamping; use
signed types (such as int, int32_t, or int64_t) or clamp/validate bounds before
subtraction. In unsigned arithmetic, subtracting a baseline from a smaller value
wraps around to a large positive value, causing subsequent clamping via SkTPin
or std::clamp to erroneously return the upper bound instead of the lower bound.

**Why.** Unsigned integer subtraction modularly wraps around when the minuend is
smaller than the subtrahend. When this wrapped large positive value is passed to
a clamp function like SkTPin or std::clamp, it clamps to the maximum limit
instead of pinning to the intended minimum bound, corrupting data and inverting
range boundaries.

## Examples

Bad:

```cpp
static constexpr uint64_t kZero = 384;
static constexpr uint64_t kRange = 510;
static constexpr uint64_t kMaxU8 = 0xff;
static constexpr uint64_t kMinU8 = 0x00;
static constexpr uint64_t kDivisor = kRange / kMaxU8;

uint64_t raw_alpha = src64[x] >> 54;
// Underflow wrap-around if raw_alpha < kZero; clamps to kMaxU8 instead of kMinU8!
uint64_t alpha = SkTPin((raw_alpha - kZero) / kDivisor, kMinU8, kMaxU8);
```

Good:

```cpp
static constexpr int64_t kZero = 384;
static constexpr int64_t kRange = 510;
static constexpr int64_t kMaxU8 = 0xff;
static constexpr int64_t kMinU8 = 0x00;
static constexpr int64_t kDivisor = kRange / kMaxU8;

int64_t raw_alpha = src64[x] >> 54;
// Signed arithmetic preserves negative values, correctly clamping to kMinU8.
int64_t alpha = SkTPin((raw_alpha - kZero) / kDivisor, kMinU8, kMaxU8);
```

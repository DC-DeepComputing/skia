---
name: skia/use-constexpr-with-k-prefix-for-constants
description: When defining constants, flags, mode selectors, sentinels, or tolerances, declare named constexpr constants with leading 'k' rather than 'f', 'g', or magic numbers
rubric: readability
severity: should_fix
---

**Rule.** Declare compile-time constants (including mode flags, selector values,
and sentinel IDs), fallback sentinels, specification scale factors, divisors,
and comparison tolerances as named constexpr (or static const for non-constexpr
sentinels) with a leading 'k' and camel-casing rather than mutable variables or
magic numbers. Do not prefix constants with 'f' (reserved for class/struct
member fields), 'g' (reserved for global variables), or Hungarian type prefixes
(use 'kInt32Max', not 'fInt32Max' or 'gLoopBins'). Prefer constexpr over plain
const for compile-time literals, express tolerance bounds via named constexpr
thresholds (e.g., 1.0f - kThreshold), and hoist loop-invariant expressions into
static constexpr with leading 'k'.

**Why.** Declaring fixed constants, mode flags, and fallback sentinels as named
constexpr or static const variables prevents accidental mutation, avoids mutable
static state, documents available states, and eliminates magic numbers. The
leading 'k' prefix conforms to Skia's style guide and immediately signals
program-duration immutability. Conversely, prefixing constants with 'f' or 'g'
misleadingly implies instance member field access or mutable global state at the
point of use.

## Examples

Bad:

```cpp
// Bad: using 'f' or 'g' prefix on a constant, or using mutable variables / magic numbers for flags
static constexpr float fInt32Max = static_cast<float>(std::numeric_limits<int32_t>::max());
static constexpr BinInfo gLoopBins[] = {{ 8, 20 }};
const size_t image_header_size = 5;
size_t partialSize = 30;
bool is_neutral = 0.95f < tmp && tmp < 1.05f;

float flags = 1;
if (needsRoundCaps) {
    flags = 2;
}
```

Good:

```cpp
// Good: named constexpr constants with leading 'k' for limits, thresholds, tables, and mode flags
static constexpr float kInt32Max = static_cast<float>(std::numeric_limits<int32_t>::max());
static constexpr BinInfo kLoopBins[] = {{ 8, 20 }};
static constexpr size_t kImageHeaderSize = 5;
constexpr size_t kPartialSize = 30;
constexpr float kPngGammaThreshold = 0.05f;
bool is_neutral = (1.0f - kPngGammaThreshold) < tmp && tmp < (1.0f + kPngGammaThreshold);

static constexpr float kIntersection_NoRoundCaps = 1;
static constexpr float kIntersection_RoundCaps = 2;
float flags = needsRoundCaps ? kIntersection_RoundCaps : kIntersection_NoRoundCaps;
```

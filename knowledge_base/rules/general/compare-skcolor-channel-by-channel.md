---
name: skia/compare-skcolor-channel-by-channel
description: When comparing SkColor values for bounds, ranges, or tolerances, compare each ARGB channel individually instead of using relational operators
rubric: correctness
severity: should_fix
---

**Rule.** Do not compare `SkColor` values using relational operators (`<`, `<=`,
`>`, `>=`) to check bounds, ranges, or tolerances. Because `SkColor` is a packed
32-bit ARGB integer, compare each color channel (A, R, G, B) individually using
accessors like `SkColorGetA()`, `SkColorGetR()`, `SkColorGetG()`, and
`SkColorGetB()`, or convert to `SkColor4f` component-wise.

**Why.** `SkColor` is a `uint32_t` typedef representing packed 8-bit ARGB
components (`(a << 24) | (r << 16) | (g << 8) | b`). Relational scalar
comparisons evaluate colors by their most-significant bits, allowing
out-of-range lower-order channels (like green and blue) to pass when
higher-order channels differ, masking rendering errors or causing spurious test
failures.

## Examples

Bad:

```cpp
// Buggy: relational comparison on packed 32-bit integer allows channel bleed
REPORTER_ASSERT(reporter, expected_min <= actual && actual <= expected_max);
```

Good:

```cpp
static bool color_in_range(SkColor c, SkColor min, SkColor max) {
    return SkColorGetA(c) >= SkColorGetA(min) && SkColorGetA(c) <= SkColorGetA(max) &&
           SkColorGetR(c) >= SkColorGetR(min) && SkColorGetR(c) <= SkColorGetR(max) &&
           SkColorGetG(c) >= SkColorGetG(min) && SkColorGetG(c) <= SkColorGetG(max) &&
           SkColorGetB(c) >= SkColorGetB(min) && SkColorGetB(c) <= SkColorGetB(max);
}
REPORTER_ASSERT(reporter, color_in_range(actual, expected_min, expected_max));
```

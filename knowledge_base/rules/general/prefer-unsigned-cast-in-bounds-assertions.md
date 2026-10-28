---
name: skia/prefer-unsigned-cast-in-bounds-assertions
description: When asserting non-negative bounds with SkASSERT, cast signed integers or enums to unsigned to verify lower and upper bounds in a single comparison
rubric: readability
severity: should_fix
---

**Rule.** When asserting that a signed integer index, size, or enum value is
within non-negative bounds in SkASSERT (or SkASSERT_RELEASE), cast the value to
unsigned (e.g. (unsigned)v < limit or (unsigned)v <= kMaxEnum) rather than
writing two separate comparisons like v >= 0 && v < limit. This tests
non-negativity and upper bounds in a single check while preventing
signed/unsigned comparison warnings against size_t or container counts.

**Why.** Casting a signed value or enum to unsigned maps negative values to
large positive numbers that exceed the upper bound, allowing a single comparison
to verify 0 <= v < limit. In debug assertions, this canonical idiom reduces
visual noise and eliminates compiler warnings when comparing signed variables
against unsigned container sizes or sizeof().

## Examples

Bad:

```cpp
// Bad: Redundant non-negativity check and verbose assertion
SkASSERT(verb >= 0 && verb < sizeof(gPtsPerVerb));
SkASSERT(index >= 0 && index < fElements.size());
```

Good:

```cpp
// Good: Single-comparison unsigned cast verifies 0 <= v < limit
SkASSERT((unsigned)verb < sizeof(gPtsPerVerb));
SkASSERT((unsigned)index < fElements.size());
```

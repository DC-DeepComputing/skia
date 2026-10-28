---
name: skia/mirror-expression-order-across-parallel-branches
description: When writing symmetric or complementary if-else branches (such as upper and lower boundary tests), align sub-expression order across branches for structural consistency
rubric: readability
severity: nit
---

**Rule.** When writing symmetric or complementary conditions across related if
and else-if branches (such as matching upper and lower boundary checks), align
the internal sequence of sub-expressions consistently across each branch. Order
parallel sub-expressions (such as outer bound thresholds followed by equality or
near-equality checks) in the same structural sequence to make differences
immediately visible.

**Why.** Inverting the order of sub-expressions across symmetric branches
obscures the structural parity of the checks, making it harder for readers to
compare complementary logic and verify boundary correctness. Consistent ordering
highlights variations and reduces cognitive overhead.

## Examples

Bad:

```cpp
// Upper bound tests near-equality before outer range, while lower bound inverts it.
if ((tValue >= 1.0 || sk_doubles_nearly_equal_ulps(tValue, 1.0)) && tValue <= 1.00005) {
    handle_upper_root();
} else if (tValue >= -0.00005 && (tValue <= 0.0 || sk_double_nearly_zero(tValue))) {
    handle_lower_root();
}
```

Good:

```cpp
// Both branches test outer range first, followed by near-equality checks.
if (tValue <= 1.00005 && (tValue >= 1.0 || sk_doubles_nearly_equal_ulps(tValue, 1.0))) {
    handle_upper_root();
} else if (tValue >= -0.00005 && (tValue <= 0.0 || sk_double_nearly_zero(tValue))) {
    handle_lower_root();
}
```

---
name: skia/prefer-initializer-list-for-max-min
description: When computing the maximum or minimum of three or more values, use std::max or std::min with an initializer list instead of nested ternary operators
rubric: readability
severity: nit
---

**Rule.** Prefer std::max({a, b, ...}) or std::min({a, b, ...}) with an
initializer list over chained ternary operators or nested two-argument
std::max/std::min calls when finding the extremum of three or more values. This
applies equally to runtime expressions and compile-time constexpr constants.

**Why.** In C++14 and later, std::max and std::min taking std::initializer_list
are constexpr and evaluate cleanly at compile time. Using initializer-list
overloads eliminates redundant nested branching and repetitive sub-expressions,
making the calculation concise, clear, and less prone to copy-paste errors.

## Examples

Bad:

```cpp
// Nested ternaries repeat expressions and obscure intent.
static constexpr int kMaxLumBits =
        B_LUM_BITS > (R_LUM_BITS > G_LUM_BITS ? R_LUM_BITS : G_LUM_BITS)
            ? B_LUM_BITS
            : (R_LUM_BITS > G_LUM_BITS ? R_LUM_BITS : G_LUM_BITS);

// Chained two-argument calls add unnecessary nesting.
int maxVal = std::max(a, std::max(b, c));
```

Good:

```cpp
// Clean, idiomatic constexpr initializer list.
static constexpr int kMaxLumBits = std::max({B_LUM_BITS, R_LUM_BITS, G_LUM_BITS});

int maxVal = std::max({a, b, c});
```

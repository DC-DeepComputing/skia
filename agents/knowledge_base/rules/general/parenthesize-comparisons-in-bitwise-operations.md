---
name: skia/parenthesize-comparisons-in-bitwise-operations
description: When combining bitwise operators (&, |, ^) with comparison, logical (&&, ||), or arithmetic operators, explicitly parenthesize sub-expressions to ensure clarity and prevent precedence bugs
rubric: readability
severity: should_fix
---

**Rule.** Always enclose bitwise expressions in explicit parentheses when
comparing their result against values or zero (e.g. (val & mask) == 0) or
combining them with logical operators (&&, ||) (e.g. (flags & kMask) && cond).
Likewise, enclose comparison operands (such as <, <=, >, >=, ==, !=) and
arithmetic sub-expressions (such as +, -) in parentheses when using them with
bitwise operators (&, |, ^) (e.g. (a > b) & (c <= d) or val & (x - 1)). For
power-of-two checks, prefer calling SkIsPow2(x).

**Why.** In C++, bitwise operators (&, |, ^) have lower precedence than
arithmetic and comparison operators, but higher precedence than logical
operators (&&, ||). Unparenthesized expressions like val & mask == 0 evaluate as
val & (mask == 0) causing logic bugs, while mixing bitwise and logical or
arithmetic operators without parentheses triggers compiler warnings (such as
GCC's -Wparentheses) and obscures evaluation intent.

## Examples

Bad:

```cpp
// Evaluates as val & (mask == 0) due to operator precedence
if (val & mask == 0) {
    return;
}
static_assert(kMaxKernelDimension & (kMaxKernelDimension - 1) == 0,
              "kMaxKernelDimension must be power of two");

// Unparenthesized bitwise expression with logical operator
if (var.modifiers().fFlags & Modifiers::kUniform_Flag && !var.type().isOpaque()) {
    return;
}

// -Wparentheses warning: comparison in operand of bitwise AND
if (!all((gap > tolerance) & abs(gap - strokeWidth) <= tolerance)) {
    return;
}

// -Wparentheses warning: arithmetic in operand of bitwise AND
SkASSERT((requiredAlignment - 1 & requiredAlignment) == 0);
```

Good:

```cpp
// Explicit parentheses around bitwise operation before equality check
if ((val & mask) == 0) {
    return;
}
static_assert((kMaxKernelDimension & (kMaxKernelDimension - 1)) == 0,
              "kMaxKernelDimension must be power of two"); // or SkIsPow2(kMaxKernelDimension)

// Explicit parentheses around bitwise operation in logical condition
if ((var.modifiers().fFlags & Modifiers::kUniform_Flag) && !var.type().isOpaque()) {
    return;
}

// Explicit parentheses around comparison operands
if (!all((gap > tolerance) & (abs(gap - strokeWidth) <= tolerance))) {
    return;
}

// Explicit parentheses around arithmetic operand
SkASSERT(((requiredAlignment - 1) & requiredAlignment) == 0);
```

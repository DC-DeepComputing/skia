---
name: skia/declare-each-variable-on-its-own-line
description: When declaring multiple initialized local variables, declare each variable on its own line instead of combining them with commas
rubric: readability
severity: nit
---

**Rule.** Declare each variable on its own line rather than combining multiple
variables or initializers into a single comma-separated declaration statement
(such as type a = expr1, b = expr2;). Outside of for-loop initialization
headers, give each variable an independent declaration line.

**Why.** Combining multiple initialized variable declarations on a single line
obscures individual types, expressions, and casts, making the code harder to
read. Placing each declaration on its own line clarifies variable types and
lifetimes, avoids confusion with pointer or reference binding, and keeps version
control diffs clean and focused.

## Examples

Bad:

```cpp
// Bad: multiple initialized variables combined in one comma-separated statement
int x = static_cast<int>(pt.x()), y = static_cast<int>(pt.y());
float minVal = compute_min(), maxVal = compute_max();
```

Good:

```cpp
// Good: each variable declared on its own line
int x = static_cast<int>(pt.x());
int y = static_cast<int>(pt.y());
float minVal = compute_min();
float maxVal = compute_max();
```

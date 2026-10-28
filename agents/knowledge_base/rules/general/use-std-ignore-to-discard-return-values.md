---
name: skia/use-std-ignore-to-discard-return-values
description: When intentionally discarding function return values or unused variables, use std::ignore = expr rather than a C-style (void) cast
rubric: readability
severity: nit
---

**Rule.** When intentionally discarding the return value of a function call
(such as in tests, benchmarks, or fuzzers) or suppressing unused variable
warnings, assign to std::ignore (`std::ignore = expr;` from `<utility>`) rather
than using a C-style `(void)expr;` cast.

**Why.** Using `std::ignore = expr;` provides a standard, idiomatic C++
mechanism that explicitly conveys the intent to drop a value. In contrast,
C-style `(void)` casts are legacy idioms that bypass type safety and are less
readable.

## Examples

Bad:

```cpp
// Bad: C-style (void) cast to discard return value or unused variable
(void)AsWinding(path);
(void)codec->incrementalDecode();
(void)unusedParam;
```

Good:

```cpp
// Good: Use std::ignore from <utility> to explicitly drop unused values
std::ignore = AsWinding(path);
std::ignore = codec->incrementalDecode();
std::ignore = unusedParam;
```

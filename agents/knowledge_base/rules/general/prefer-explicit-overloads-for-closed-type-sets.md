---
name: skia/prefer-explicit-overloads-for-closed-type-sets
description: When defining operations over a closed set of types where some require safety guards, write explicit overloads rather than unconstrained template fallbacks
rubric: design
severity: should_fix
---

**Rule.** Do not use an unconstrained fallback template (`template <typename
T>`) when implementing operations over a closed set of types where specific
types require special safety handling (such as integer division zero-checks or
overflow guards). Define explicit overloads for each supported type instead so
that unsupported or unsafe types produce a compile-time error.

**Why.** Unconstrained fallback templates allow unsupported types to silently
match and bypass necessary safety checks, such as division-by-zero or overflow
protection. When the supported domain is a fixed, closed set of types, explicit
overloads provide static type safety, ensure required guards are tailored to
each type, and prevent unintended instantiations.

## Examples

Bad:

```cpp
// Bad: Unconstrained fallback template silently matches unintended types
SI void div_fn(I32* dst, I32* src) { /* zero and INT32_MIN overflow guards */ }
SI void div_fn(U32* dst, U32* src) { /* zero guards */ }
template <typename T>
SI void div_fn(T* dst, T* src) {
    *dst /= *src;
}
```

Good:

```cpp
// Good: Explicit overloads for the closed set of supported types (I32, U32, F)
SI void div_fn(I32* dst, I32* src) { /* zero and INT32_MIN overflow guards */ }
SI void div_fn(U32* dst, U32* src) { /* zero guards */ }
SI void div_fn(F* dst, F* src) {
    *dst /= *src;
}
```

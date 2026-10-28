---
name: skia/api/mark-value-type-constructors-constexpr
description: When defining constructors for lightweight value types, geometry structs, or wrapper classes that perform simple member initialization, mark them constexpr to enable compile-time evaluation
rubric: readability
severity: should_fix
---

**Rule.** When defining constructors for lightweight value types, geometry
types, or wrapper classes that perform simple member initialization, mark them
constexpr. Defining these constructors constexpr allows instances to be
evaluated and used in compile-time contexts, such as static assertions,
constexpr helper functions, and constexpr constants.

**Why.** Omitting constexpr on simple member-initializing constructors forces
instances to be constructed at runtime, preventing their use in constexpr
expressions, static initializers, or compile-time template arguments. Marking
constructors constexpr enables compile-time evaluation with zero runtime
overhead.

## Examples

Bad:

```cpp
template <typename T, typename E>
class SkResult {
public:
    SkResult(const T& val) : fStorage(val) {}
    SkResult(E err) : fStorage(err) {}
    ...
};
```

Good:

```cpp
template <typename T, typename E>
class SkResult {
public:
    constexpr SkResult(const T& val) : fStorage(val) {}
    constexpr SkResult(E err) : fStorage(err) {}
    ...
};
```

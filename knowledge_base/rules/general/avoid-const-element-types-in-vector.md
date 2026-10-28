---
name: skia/avoid-const-element-types-in-vector
description: When declaring containers like std::vector, do not use const element types; store non-const types and expose read-only views via SkSpan<const T>
rubric: readability
severity: blocking
---

**Rule.** Do not use `const` element types in `std::vector` or standard dynamic
containers (such as `std::vector<const T>`). Define containers with non-const
value types (`std::vector<T>`) and return non-owning views such as `SkSpan<const
T>` or `const std::vector<T>&` when callers require read-only access.

**Why.** Standard dynamic containers require element types to be assignable and
erasable. Qualifying container elements as `const` prevents copy and move
assignment, triggering compiler errors or undefined behavior across C++ standard
library implementations. Storing non-const element types and exposing
`SkSpan<const T>` or const container references provides compile-time
immutability for callers without violating container requirements.

## Examples

Bad:

```cpp
std::vector<const SkRuntimeEffect::ChildPtr> fChildren;

std::vector<const SkRuntimeEffect::ChildPtr> children() const {
    return fChildren;
}
```

Good:

```cpp
std::vector<SkRuntimeEffect::ChildPtr> fChildren;

SkSpan<const SkRuntimeEffect::ChildPtr> children() const {
    return fChildren;
}
```

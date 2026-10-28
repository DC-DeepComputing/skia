---
name: skia/general-avoid-const-container-elements
description: When declaring standard or dynamic containers (such as std::vector or TArray), do not use const value types; store non-const T and expose read-only access via SkSpan<const T>
rubric: readability
severity: blocking
---

**Rule.** Do not use const element types when declaring standard or dynamic
containers (such as std::vector<const T> or skia_private::TArray<const T>).
Standard containers require non-const, erasable value types; to provide
read-only access to container elements, store non-const T and expose read-only
views via SkSpan<const T> or const references.

**Why.** Standard library and dynamic container implementations require element
types to be complete, non-const, and erasable; instantiating containers with
const element types (such as std::vector<const T>) is ill-formed and causes
build breaks under standard C++ compliance. Storing non-const elements while
returning SkSpan<const T> or const references cleanly enforces read-only access
for callers without invalidating container contracts.

## Examples

Bad:

```cpp
// Ill-formed: std::vector requires non-const value types
std::vector<const SkRuntimeEffect::ChildPtr> fChildren;

std::vector<const SkRuntimeEffect::ChildPtr> children() const {
    return fChildren;
}
```

Good:

```cpp
// Store non-const elements and expose read-only access via SkSpan
std::vector<SkRuntimeEffect::ChildPtr> fChildren;

SkSpan<const SkRuntimeEffect::ChildPtr> children() const {
    return fChildren;
}
```

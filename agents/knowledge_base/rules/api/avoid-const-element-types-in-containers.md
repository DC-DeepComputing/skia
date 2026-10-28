---
name: skia/api/avoid-const-element-types-in-containers
description: When declaring containers like std::vector or TArray, do not use const element types; store non-const elements and expose read-only views via SkSpan<const T>
rubric: readability
severity: should_fix
---

**Rule.** Do not use const element types when declaring standard or custom
containers (such as std::vector<const T> or TArray<const T>). Store non-const
element types (std::vector<T>) and enforce immutability by exposing read-only
views via SkSpan<const T> or const member accessors.

**Why.** Standard containers require element types to be non-const (Erasable and
MoveAssignable); instantiating containers with const element types violates
standard C++ allocator requirements and prevents moving or reassigning elements.
Storing non-const elements while exposing SkSpan<const T> or const references
provides compile-time read-only safety to callers without violating container
invariants.

## Examples

Bad:

```cpp
// Bad: std::vector<const T> is invalid C++ and breaks allocator requirements
class RuntimeShader {
    std::vector<const SkRuntimeEffect::ChildPtr> fChildren;
};
```

Good:

```cpp
// Good: Store non-const elements and expose read-only views via SkSpan<const T>
class RuntimeShader {
public:
    SkSpan<const SkRuntimeEffect::ChildPtr> children() const { return fChildren; }

private:
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};
```

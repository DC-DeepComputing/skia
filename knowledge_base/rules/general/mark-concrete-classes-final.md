---
name: skia/mark-concrete-classes-final
description: When declaring concrete leaf implementation classes or internal types deriving from interfaces, mark the class declaration final to prevent unintended inheritance and aid devirtualization
rubric: readability
severity: should_fix
---

**Rule.** When defining concrete leaf implementation classes (such as FooImpl
classes implementing an interface) that are not intended to be further
subclassed, mark the class declaration final (e.g., class FooImpl final : public
Foo). Do not leave concrete leaf classes unconstrained when further inheritance
is neither designed nor supported.

**Why.** Marking concrete leaf classes as final clarifies design intent by
forbidding further inheritance, avoids object slicing hazards, and enables
compiler devirtualization of member function calls.

## Examples

Bad:

```cpp
// Concrete leaf implementation class left open to inheritance
class AgtmImpl : public Agtm {
public:
    void applyGain() override;
};
```

Good:

```cpp
// Marked final to prevent subclassing and enable devirtualization
class AgtmImpl final : public Agtm {
public:
    void applyGain() override;
};
```

---
name: skia/general-return-builders-by-value
description: When defining or returning builder classes, provide public constructors and value semantics with Builder& chaining instead of heap-allocating via std::unique_ptr
rubric: design
severity: should_fix
---

**Rule.** Design builder classes with public constructors and value semantics,
and return Builder& from fluent chaining methods rather than heap-allocating via
static Make() methods returning std::unique_ptr<Builder>. When instantiating or
returning builders from helper or factory functions, return concrete builder
types by value so callers can use auto and invoke methods directly on the stack
without pointer indirection, even across conditionally compiled (#ifdef)
branches.

**Why.** Builders are transient configuration helpers meant to assemble state on
the stack. Wrapping them in std::unique_ptr with private constructors introduces
unnecessary dynamic allocation overhead and pointer indirection (->), whereas
value semantics with Builder& chaining simplify call sites, avoid allocator
churn, and allow clean deduction via auto.

## Examples

Bad:

```cpp
// Heap-allocated builder with pointer chaining and factory returning unique_ptr:
class DisplayParamsBuilder {
public:
    static std::unique_ptr<DisplayParamsBuilder> Make();
    DisplayParamsBuilder* setScale(float s);
private:
    DisplayParamsBuilder();
};
static std::unique_ptr<DisplayParamsBuilder> make_builder() {
    return DisplayParamsBuilder::Make();
}
```

Good:

```cpp
// Value-semantic builder with reference chaining and helper returning by value:
class DisplayParamsBuilder {
public:
    DisplayParamsBuilder();
    DisplayParamsBuilder& setScale(float s);
};
static DisplayParamsBuilder make_builder() {
    return DisplayParamsBuilder();
}
```

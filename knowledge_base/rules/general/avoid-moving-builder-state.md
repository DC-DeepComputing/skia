---
name: skia/avoid-moving-builder-state
description: When extracting constructed objects from builder classes, move state via detach() rather than cloning, and reserve copying for explicitly reusable build() methods
rubric: safety
severity: should_fix
---

**Rule.** In builder classes, transfer ownership of constructed state via
std::move from detach() (or build()) rather than performing deep copies or
calling .clone(), unless builder reuse across multiple calls is an explicit
requirement. When destructive single-use extraction is intended, prefer naming
the method detach() rather than build() and assert that internal state is
non-null before moving. Conversely, if a builder must remain reusable across
repeated calls, make build() const and return a copy or clone without emptying
internal state.

**Why.** Builder classes typically construct an object incrementally for a
single consumer; cloning or deep-copying internal state incurs unnecessary heap
allocations and buffer duplication. Conversely, calling std::move on internal
builder state empties the pointer or container, turning an idempotent build()
method into a destructive operation where subsequent calls return null or crash.
Using detach() with non-null assertions explicitly communicates single-use
ownership transfer and prevents null dereferences.

## Examples

Bad:

```cpp
// Bad: Redundant deep clone when single-use extraction is intended
std::unique_ptr<DisplayParams> DisplayParamsBuilder::build() {
    return fDisplayParams ? fDisplayParams->clone() : nullptr;
}

// Bad: Destructive std::move in a nominally reusable build() method without checks
std::unique_ptr<DisplayParams> DisplayParamsBuilder::build() {
    return std::move(fDisplayParams);
}
```

Good:

```cpp
// Good: Destructive single-use extraction moves state and asserts non-null
std::unique_ptr<DisplayParams> DisplayParamsBuilder::detach() {
    SkASSERT_RELEASE(fDisplayParams);
    return std::move(fDisplayParams);
}

// Good: Reusable builder explicitly cloned/copied via const build()
std::unique_ptr<DisplayParams> DisplayParamsBuilder::build() const {
    return fDisplayParams ? fDisplayParams->clone() : nullptr;
}
```

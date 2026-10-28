---
name: skia/assert-preconditions-over-silent-fallback-values
description: When an operation or accessor is invalid for certain object states, assert the precondition with SkASSERT rather than silently returning dummy fallback values
rubric: correctness
severity: should_fix
---

**Rule.** When an operation, method, or accessor is invalid for specific object
states, configurations, or representations, assert the precondition using
SkASSERT rather than silently returning a dummy or zero fallback value. Do not
allow invalid invocations to proceed or return default values that conceal
caller contract violations.

**Why.** Silently returning a zero, dummy, or fallback value when an operation
is invalid for an object's state masks caller logic errors and allows execution
to proceed under corrupt or unsupported assumptions. Asserting the precondition
with SkASSERT enforces the API contract, causes contract violations to fail fast
in debug builds and test suites, and makes state prerequisites explicit to
maintainers.

## Examples

Bad:

```cpp
// Bad: Silently returns 0 when called on an unsupported or invalid state
size_t slotCount() const override {
    return fSlotCount; // fSlotCount is 0 if fContainsUnsizedArray is true
}
```

Good:

```cpp
// Good: Asserts the precondition so invalid access fails fast in debug builds
size_t slotCount() const override {
    SkASSERT(!fContainsUnsizedArray);
    return fSlotCount;
}
```

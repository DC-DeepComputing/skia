---
name: skia/gpu/name-internal-state-reset-helpers
description: When declaring internal state-reset helpers in GPU classes, name them with reset or resetFor<Operation> rather than GPU pipeline operations like discard
rubric: readability
severity: nit
---

**Rule.** Do not name internal state-reset helpers after GPU hardware or
pipeline operations (such as `internalDiscard`) when they only reset internal
bookkeeping or tracking state; name them using explicit reset terminology, such
as `internalReset()` or `resetFor<Operation>()` (e.g.,
`resetForClearOrDiscard()`). When a reset routine prepares state for multiple
subsequent operations (like clear and discard), reflect all applicable
operations in the name.

**Why.** In GPU subsystems, terms like 'discard' or 'clear' strongly imply
hardware operations on texture contents or configuring render pass load/store
ops (e.g., LoadOp::kDiscard). Using them for internal bookkeeping helpers
obscures that only CPU-side state (such as atlas allocators or pending draws) is
being refreshed, especially when the reset is shared across multiple call sites.

## Examples

Bad:

```cpp
class DrawContext {
    // Misleading: implies GPU texture discard rather than resetting CPU tracking
    void internalDiscard();
};
```

Good:

```cpp
class DrawContext {
    // Explicitly describes resetting internal tracking for either operation
    void resetForClearOrDiscard();
};
```

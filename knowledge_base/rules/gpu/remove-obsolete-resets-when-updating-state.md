---
name: skia/gpu/remove-obsolete-resets-when-updating-state
description: When refactoring stateful transition methods like newPipelineState to compute state rather than resetting it, delete obsolete reset assignments and assert post-flush invariants immediately
rubric: correctness
severity: should_fix
---

**Rule.** When refactoring draw batching state transition methods (such as
DrawWriter::newPipelineState) to compute and retain state rather than resetting
it, remove obsolete reset assignments and outdated comments so they do not
overwrite newly calculated values. Additionally, preserve still-valid invariant
checks by asserting them (such as SkASSERT(fPendingCount == 0)) immediately
following the flush or operation that establishes the invariant.

**Why.** Leaving obsolete reset assignments in place after computing updated
state silently clobbers the new values and introduces logic bugs. Placing
post-flush assertions immediately adjacent to the flush call localizes invariant
verification to where it is established and prevents state regressions across
subsequent calculations.

## Examples

Bad:

```cpp
void newPipelineState(PrimitiveType type, ...) {
    this->flush();
    fPendingBase = computeAlignedBase(...);

    // NOTE: resetting pending base is sufficient to redo bindings...
    fPendingBase = 0;
    SkASSERT(fPendingCount == 0);
}
```

Good:

```cpp
void newPipelineState(PrimitiveType type, ...) {
    this->flush();
    SkASSERT(fPendingCount == 0);

    fPendingBase = computeAlignedBase(...);
}
```

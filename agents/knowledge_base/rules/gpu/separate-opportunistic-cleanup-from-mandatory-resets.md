---
name: skia/gpu/separate-opportunistic-cleanup-from-mandatory-resets
description: When organizing cleanup routines or resets, separate opportunistic maintenance calls from mandatory cleanup blocks and document their distinct purpose and frequency
rubric: readability
severity: nit
---

**Rule.** Do not place opportunistic maintenance or periodic resource processing
calls under comment blocks that describe mandatory state cleanups or invariant
resets. Separate opportunistic routines into distinct statements and document
their specific motivations and triggers.

**Why.** Grouping opportunistic calls under comments designating mandatory
cleanups misleads readers into assuming the operation is required for
correctness, state consistency, or error recovery. Documenting opportunistic
tasks separately clarifies why they run and when their execution is expected.

## Examples

Bad:

```cpp
// Remaining cleanup that must always happen regardless of success or failure
fResourceProvider->forceProcessReturnedResources();
fRuntimeEffectDict->reset();
```

Good:

```cpp
// Process the return queue at least once to keep it from growing too large.
fResourceProvider->forceProcessReturnedResources();

// Remaining cleanup that must always happen regardless of success or failure
fRuntimeEffectDict->reset();
```

---
name: skia/modules/consume-active-builder-before-member-assignment
description: When refactoring path mutation to SkPathBuilder, consume intermediate geometry from the active builder via detach() or snapshot(), not unfinalized members
rubric: correctness
severity: blocking
---

**Rule.** When refactoring path construction from in-place mutation of an SkPath
member to a local SkPathBuilder, ensure intermediate reads and downstream
builder operations consume from the active SkPathBuilder (e.g. via detach() or
snapshot()) rather than the destination SkPath member. Do not read the
destination member before finalization, as it remains empty or stale while the
local builder accumulates.

**Why.** Deferred assignment to an SkPath destination member leaves it holding
uninitialized or stale state while the local SkPathBuilder accumulates
intermediate geometry. Reading from the unfinalized member feeds empty or stale
data into downstream consumers like SkOpBuilder, causing silent visual
regressions.

## Examples

Bad:

```cpp
SkPathBuilder merger;
// ... accumulate paths into merger ...
if (!in_builder) {
    builder.add(fMerged.snapshot(), kUnion_SkPathOp); // Bug: fMerged is stale/empty
    in_builder = true;
}
fMerged = in_builder ? builder.detach() : merger.detach();
```

Good:

```cpp
SkPathBuilder merger;
// ... accumulate paths into merger ...
if (!in_builder) {
    builder.add(merger.detach(), kUnion_SkPathOp); // Consumes accumulated paths from merger
    in_builder = true;
}
fMerged = in_builder ? builder.detach() : merger.detach();
```

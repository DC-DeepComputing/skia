---
name: skia/update-local-staging-variable-before-commit
description: When performing fallible operations (allocations, parsing, I/O), defer mutating object state or stage updates in local variables until operations succeed rather than rolling back
rubric: correctness
severity: blocking
---

**Rule.** When performing fallible operations (such as memory allocations,
parsing, resource creation, or I/O), do not prematurely mutate object state,
member fields, or persistent containers and attempt to roll them back on
failure. Defer state updates until all fallible steps succeed, or stage pending
state changes and unvalidated elements in local temporary variables across
intermediate branches and commit them to member fields or collections only upon
final success.

**Why.** Mutating member fields or appending unvalidated elements to persistent
containers before fallible operations succeed leaves the object corrupted or in
an inconsistent state if an operation fails. Relying on error-path rollbacks is
fragile and prone to desynchronization when branches change. Staging state
locally or deferring mutations until fallible steps succeed ensures
transactional consistency without complex rollback logic.

## Examples

Bad:

```cpp
// 1. Mutating member state before a fallible allocation and rolling back
fBytesWrittenBeforeTail += fTail->written();
Block* block = (Block*)sk_malloc_canfail(sizeof(Block) + size);
if (!block) {
    if (fTail) { fBytesWrittenBeforeTail -= fTail->written(); }
    return false;
}

// 2. Mutating mode flags before resource initialization succeeds
fXformMode = kSwizzleColor_XformMode;
fSwizzler = SkSwizzler::Make(...);
if (!fSwizzler) { return kUnimplemented; }

// 3. Appending unvalidated elements to persistent container before parsing
fFrames.emplace_back(id, info.alpha());
if (reader.has_fctl_chunk()) {
    return this->setLastFrameInfoFromCurrentFctlChunk(reader);
}
```

Good:

```cpp
// 1. Deferring member update until fallible allocation succeeds
Block* block = (Block*)sk_malloc_canfail(sizeof(Block) + size);
if (!block) { return false; }
if (fTail) { fBytesWrittenBeforeTail += fTail->written(); }

// 2. Staging state in local variable and committing only on success
XformMode newXformMode = kSwizzleColor_XformMode;
fSwizzler = SkSwizzler::Make(...);
if (!fSwizzler) { return kUnimplemented; }
fXformMode = newXformMode;

// 3. Validating into local temporary before appending to container
if (reader.has_fctl_chunk()) {
    PngFrame frame(id, info.alpha());
    if (this->setFrameInfoFromCurrentFctlChunk(reader, &frame) == SkCodec::kSuccess) {
        fFrames.push_back(std::move(frame));
    }
}
```

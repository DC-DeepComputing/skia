---
name: skia/gpu/assert-nested-scopes-closed-at-checkpoints
description: When rewinding, marking checkpoints, or finalizing buffer builders with nested scopes, assert that nested structures are closed rather than silently clearing tracking state
rubric: correctness
severity: should_fix
---

**Rule.** In stateful buffer managers and builders that track nested scopes
(such as beginStruct() and endStruct()), assert with SkASSERT that all nested
structures are closed (e.g., verifying that active struct alignment or substruct
offset tracking is reset to zero or sentinel values) when rewinding storage,
recording checkpoints, or finalizing top-level allocations. Do not silently
reset, clear, or bypass nested tracking state during rewind or boundary
transitions.

**Why.** Boundary transitions like rewinding offsets or marking checkpoints are
invalid while inside an open nested structure. Silently clearing nested state
masks caller contract violations (such as missing endStruct() calls) and allows
corrupted layout state to propagate, whereas asserting scope closure catches
state machine desynchronization immediately at debug time.

## Examples

Bad:

```cpp
void UniformManager::rewindToMark() {
    fStorage.resize(fEndPaintOffset);
    SkDEBUGCODE(fOffsetCalculator = fMarkedOffsetCalculator);
    // BAD: Silently clearing nested struct state masks unclosed structs from callers.
    fSubstructStartingOffset = -1;
    fStructBaseAlignment = 0;
}
```

Good:

```cpp
void UniformManager::rewindToMark() {
    fStorage.resize(fEndPaintOffset);
    SkDEBUGCODE(fOffsetCalculator = fMarkedOffsetCalculator);
    // GOOD: Assert that all nested structures were properly closed before rewinding.
    SkASSERT(fSubstructStartingOffset == -1);
    SkASSERT(fStructBaseAlignment == 0);
}
```

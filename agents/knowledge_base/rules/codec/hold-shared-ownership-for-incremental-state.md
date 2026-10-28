---
name: skia/codec/hold-shared-ownership-for-incremental-state
description: When managing state across incremental decode operations (onStartIncrementalDecode and onIncrementalDecode), retain explicit shared ownership of required resources directly rather than storing container indices
rubric: safety
severity: should_fix
---

**Rule.** When managing state across multi-step or incremental decode operations
(such as `onStartIncrementalDecode` and `onIncrementalDecode`), retain explicit
shared ownership (`sk_sp<T>`) of required buffers or resources directly on the
session state. Do not store an integer index into an underlying collection to
look up resources on subsequent decode steps.

**Why.** Storing container indices requires repetitive defensive bounds checks
and container lookups across incremental calls, and risks use-after-free or
out-of-bounds access if the underlying container is reallocated or modified.
Holding an explicit `sk_sp` keeps the required resource alive for the duration
of the incremental operation and decouples the session state from parent
container layout.

## Examples

Bad:

```cpp
// In decoder header:
int fCurrCodecIndex = -1;

// In onStartIncrementalDecode:
fCurrCodecIndex = codecIndex;

// In onIncrementalDecode:
if (fCurrCodecIndex >= 0 &&
    fCurrCodecIndex < static_cast<int>(fBmpEntryData.size()) &&
    fBmpEntryData[fCurrCodecIndex]) {
    const auto& entryData = fBmpEntryData[fCurrCodecIndex];
    ...
}
```

Good:

```cpp
// In decoder header:
sk_sp<const SkData> fIncrementalBmpEntryData;

// In onStartIncrementalDecode:
fIncrementalBmpEntryData = fEmbeddedImages[codecIndex].fBmpEntryData;

// In onIncrementalDecode:
if (fIncrementalBmpEntryData) {
    const auto& entryData = fIncrementalBmpEntryData;
    ...
}
```

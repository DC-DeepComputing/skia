---
name: skia/general-avoid-locking-immutable-data
description: When declaring or accessing immutable or read-only data initialized at construction, omit mutexes, spinlocks, or SK_GUARDED_BY annotations since concurrent reads are thread-safe
rubric: safety
severity: should_fix
---

**Rule.** Do not protect immutable or read-only data with mutexes or spinlocks
(nor annotate fields with SK_GUARDED_BY) when the data is fully initialized
during object construction and never modified thereafter. In C++, concurrent
reads on immutable data are inherently thread-safe; initialize such fields
eagerly in constructor initializer lists and access them directly without
synchronization.

**Why.** Guarding immutable or read-only data with a lock introduces unnecessary
locking overhead, code complexity, and thread contention. Because immutable data
is never written to after construction, concurrent reader threads cannot race,
making synchronization redundant.

## Examples

Bad:

```cpp
SkTDArray<uint8_t> fMSAAMaskLUT SK_GUARDED_BY(fSpinLock);

const SkTDArray<uint8_t>* getMSAA8MaskLUT() SK_EXCLUDES(fSpinLock) {
    SkAutoSpinlock lock{fSpinLock};
    SkASSERT(!fMSAAMaskLUT.empty());
    return &fMSAAMaskLUT;
}
```

Good:

```cpp
SkTDArray<uint8_t> fMSAAMaskLUT;

SkSpan<const uint8_t> getMSAA8MaskLUT() const {
    return fMSAAMaskLUT;
}
```

---
name: skia/avoid-duplicate-counters-for-container-sizes
description: When tracking element counts or capacity in classes, query the underlying container or resource size directly instead of maintaining duplicate counter member variables
rubric: readability
severity: should_fix
---

**Rule.** Do not maintain a separate member variable or counter to track the
element count, size, or consumed slots of an object or buffer when an underlying
container already records them. Query the container's count or size (.count(),
.size()) directly, or through an accessor on the resource, as the single source
of truth.

**Why.** Maintaining redundant count or slot members alongside collections that
already track their own size introduces duplicate state that can easily
desynchronize during buffer resets, reallocations, or error handling. Relying
directly on the container or child resource as the single source of truth
eliminates unnecessary bookkeeping variables and simplifies invariants.

## Examples

Bad:

```cpp
class IntrinsicConstantsManager {
    IntrinsicBuffer* fCurrentBuffer = nullptr;
    int fSlotsUsed = 0;

    void add(const IntrinsicValues& values, uint32_t stride) {
        if (!fCurrentBuffer || fSlotsUsed == IntrinsicBuffer::kNumSlots) {
            fCurrentBuffer = new IntrinsicBuffer();
            fSlotsUsed = 0;
        }
        uint32_t offset = (fSlotsUsed++) * stride;
        fCurrentBuffer->trackIntrinsic(values, offset);
    }
};
```

Good:

```cpp
class IntrinsicBuffer {
public:
    int slotsUsed() const { return fCachedIntrinsicValues.count(); }
};

class IntrinsicConstantsManager {
    IntrinsicBuffer* fCurrentBuffer = nullptr;

    void add(const IntrinsicValues& values, uint32_t stride) {
        if (!fCurrentBuffer || fCurrentBuffer->slotsUsed() == IntrinsicBuffer::kNumSlots) {
            fCurrentBuffer = new IntrinsicBuffer();
        }
        uint32_t offset = fCurrentBuffer->slotsUsed() * stride;
        fCurrentBuffer->trackIntrinsic(values, offset);
    }
};
```

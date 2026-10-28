---
name: skia/api/enforce-decode-memory-budget-inline
description: When bounding image decoder memory usage, enforce cumulative limits inline, initialize budgets across session entry points, and decrement remaining budget to prevent overflow
rubric: readability
severity: blocking
---

**Rule.** Do not expose speculative pre-flight memory estimation methods (such
as `estimateMaxAllocation`) on decoder interfaces that duplicate internal setup
logic; enforce cumulative memory limits inline during decoding instead.
Furthermore, initialize execution-scoped budgets (`fDecodeBudget`) across all
session entry points (such as `startScanlineDecode`, `getPixels`, and
`getImage`), defaulting to unbounded (`SIZE_MAX`) when unset. When enforcing
limits, track the remaining budget decremented per allocation rather than
accumulating usage against a maximum to eliminate addition overflow hazards and
simplify state.

**Why.** Pre-flight estimation methods duplicate format-specific decoder setup
paths and risk code drift, while failing to initialize execution-scoped budgets
across all entry points leaves them at zero, causing spurious `kOutOfMemory`
failures. Tracking accumulated usage against a maximum limit requires integer
overflow guards on addition (`used + requested > max`), whereas decrementing a
remaining budget directly simplifies the guard to `requested > remaining` and
reduces tracking to a single counter.

## Examples

Bad:

```cpp
// Bad: Exposing speculative pre-flight estimation, failing to initialize budget
// across all entry points, and accumulating usage with addition overflow hazards:
class SkCodec {
public:
    virtual size_t estimateMaxAllocation(SkISize dims) const = 0;
};

SkCodec::Result SkCodec::startScanlineDecode(const SkImageInfo& info,
                                            const Options* options) {
    // Bug: fDecodeBudget remains 0, causing subsequent allocations to fail.
    return this->onStartScanlineDecode(info, options);
}

bool SkCodec::allocateFromBudget(size_t numBytes) {
    if (numBytes > SIZE_MAX - fBytesAllocatedForDecode) SK_UNLIKELY {
        return false;
    }
    if (fBytesAllocatedForDecode + numBytes > fMaxBytesForDecode) SK_UNLIKELY {
        return false;
    }
    fBytesAllocatedForDecode += numBytes;
    return true;
}
```

Good:

```cpp
// Good: Enforce cumulative limits inline, initialize budgets across entry points,
// and track remaining budget decremented per allocation:
struct Options {
    size_t fMaxDecodeMemory = 0;
};

SkCodec::Result SkCodec::startScanlineDecode(const SkImageInfo& info,
                                            const Options* options) {
    fDecodeBudget = (options && options->fMaxDecodeMemory)
                        ? options->fMaxDecodeMemory
                        : SIZE_MAX;
    return this->onStartScanlineDecode(info, options);
}

bool SkCodec::allocateFromBudget(size_t numBytes) {
    if (numBytes > fDecodeBudget) SK_UNLIKELY {
        return false;
    }
    fDecodeBudget -= numBytes;
    return true;
}
```

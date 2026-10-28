---
name: skia/api/document-mutable-atomic-caching-fields
description: When declaring mutable member variables for deferred caching or lazy evaluation, document the lazy initialization, identifying populating methods or thread-safe atomic caching rationale
rubric: readability
severity: should_fix
---

**Rule.** When declaring member variables as `mutable` (including `mutable
std::atomic<T>`) for deferred caching or lazy evaluation, document the lazy
initialization in an inline comment and identify the populating method(s) (e.g.,
`lazily allocated by makeColorFilter()`). If deferred computation or caching is
pending runtime benchmarking or full implementation, explain the thread-safe
caching rationale and future evaluation plan in an inline comment or attributed
TODO.

**Why.** Mutable members bypass C++ const-correctness guarantees and can raise
thread-safety, lifecycle, or dead-code questions in otherwise immutable classes.
Documenting lazy initialization and naming the populating method(s) or
thread-safe caching rationale clarifies why the field must be mutable rather
than const or eagerly initialized, preventing premature simplification or
removal.

## Examples

Bad:

```cpp
class SkPathData {
    uint8_t                      fSegmentMask;
    mutable std::atomic<uint8_t> fConvexity;
};

// Cached SkImage containing control point values for color filter.
mutable sk_sp<SkImage> fGainCurvesXYM;
```

Good:

```cpp
class SkPathData {
    uint8_t                      fSegmentMask;
    // Thread-safe lazy caching: evaluate lazily if runtime benchmarks show deferring
    // convexity calculation is worthwhile.
    mutable std::atomic<uint8_t> fConvexity;
};

// SkImage containing control point values for color filter. Lazily allocated by
// makeColorFilter().
mutable sk_sp<SkImage> fGainCurvesXYM;
```

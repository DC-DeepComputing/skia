---
name: skia/prefer-skspan-over-pointer-and-count
description: When passing, returning, or iterating over contiguous buffers, use SkSpan instead of pointer/array-count pairs, raw pointer views, or const std::vector references
rubric: readability
severity: should_fix
---

**Rule.** Prefer accepting or returning contiguous sequences and buffer views as
SkSpan<T> (or SkSpan<const T>) rather than separate pointer- or array-count
arguments (such as T* ptr, int count or const T[] arr, int count), raw pointer
views that force callers to reconstruct spans with magic lengths, or concrete
container references (such as const std::vector<T>&). When touching or modifying
existing signatures that accept pointer/array and count pairs, take the
opportunity to span-ify the parameters. When an existing pointer-and-count API
cannot be modified or when iterating over raw pointers, wrap the pointer and
count in a temporary SkSpan(ptr, count) to enable range-based for loops without
manual index tracking.

**Why.** Taking or returning SkSpan encapsulates the element count with the
pointer, preventing length mismatches, eliminating redundant manual {ptr, count}
constructions at call sites, and decoupling APIs from caller container
implementations. For raw pointer buffers, SkSpan enables clean range-based for
loops and bounds safety without manual index tracking.

## Examples

Bad:

```cpp
// 1. Concrete container references for read-only inspection:
bool ValidateSPIRV(ErrorReporter&, const std::vector<uint32_t>&);

// 2. Separate pointer/array and count parameters in signatures:
void extend_pts(SkPoint* pts, int ptCount);
virtual FilterReturn filterRectsToNine(const SkRect rects[], int count, ...) const;

// 3. Helper function returning raw pointer view without length:
static const SkPoint* rect_points(const SkRect& r) {
    return reinterpret_cast<const SkPoint*>(&r);
}
// Call site forces manual span reconstruction:
fCTM->mapPoints({rect_points(devRect), 2}, {rect_points(paintRect), 2});

// 4. Manual indexed loop over pointer and count:
for (int i = 0; i < count; i++) {
    process(pts[i]);
}
```

Good:

```cpp
// 1. Accept SkSpan to decouple from caller container representations:
bool ValidateSPIRV(ErrorReporter&, SkSpan<const uint32_t>);

// 2. Accept SkSpan directly in function signatures:
void extend_pts(SkSpan<SkPoint> pts);
virtual FilterReturn filterRectsToNine(SkSpan<const SkRect> rects, ...) const;

// 3. Return SkSpan from view helpers to bundle element count:
static SkSpan<const SkPoint> rect_points(const SkRect& r) {
    return {reinterpret_cast<const SkPoint*>(&r), 2};
}
// Call site passes span directly:
fCTM->mapPoints(rect_points(devRect), rect_points(paintRect));

// 4. Wrap pointer and count in a temporary SkSpan for range-for loops:
for (const auto& pt : SkSpan(pts, count)) {
    process(pt);
}
```

---
name: skia/api/prefer-skto-over-static-cast
description: When converting numeric values, use Skia checked helpers like SkToInt instead of static_cast, but avoid SkToInt on unvalidated inputs in bounds guards
rubric: correctness
severity: should_fix
---

**Rule.** When converting or narrowing numeric values (such as container sizes
or integer types), use Skia's checked conversion helpers (such as SkToInt,
SkToSizeT, SkToUInt, SkToU32, or SkTo<T>) instead of raw static_cast<T>, or
match types to avoid casting. However, do not use asserting narrowing helpers
like SkToInt on unvalidated inputs in bounds checks or early-exit guards; widen
the trusted container size (e.g., using SkToSizeT) instead to prevent debug
assertion crashes.

**Why.** In Skia, include/private/base/SkTo.h conversion helpers wrap
static_cast with debug checks (SkASSERT(SkTFitsIn<D>(s))), catching unexpected
overflow and signedness mismatches without raw cast boilerplate. However,
because these helpers assert, downcasting unvalidated caller inputs inside
early-exit or bounds-checking guards causes debug builds to crash rather than
gracefully taking the bailout path; widening the trusted non-negative bound
avoids the assertion.

## Examples

Bad:

```cpp
// Bad: raw static_cast performs unchecked narrowing conversions.
for (int i = 0; i < static_cast<int>(fEmbeddedImages.size()); ++i) {
    processImage(i);
}
uint32_t count = static_cast<uint32_t>(rawCount);

// Bad: asserting downcast on unvalidated input in bounds check crashes debug builds.
if (start >= end || SkToInt(start) > fUTF8IndexForUTF16Index.size() || end == 0) {
    return results;
}
```

Good:

```cpp
// Good: Skia checked helpers assert in debug builds that values fit.
for (int i = 0; i < SkToInt(fEmbeddedImages.size()); ++i) {
    processImage(i);
}
uint32_t count = SkToU32(rawCount);

// Good: widen the trusted non-negative container size to compare safely without asserting.
if (start >= end || start > SkToSizeT(fUTF8IndexForUTF16Index.size()) || end == 0) {
    return results;
}
```

---
name: skia/factor-out-helpers-over-weakening-assertions
description: When reusing a class's calculation or formatting logic in other subsystems, extract shared helpers rather than weakening constructor precondition assertions to allow dummy instances
rubric: design
severity: should_fix
---

**Rule.** Do not weaken or remove constructor precondition assertions (such as
SkASSERT(ptr)) to permit instantiating dummy or partially initialized objects
solely to call secondary helper methods. Extract the shared calculation,
formatting, or evaluation logic into a dedicated helper class, struct, or static
function that can be used independently without constructing the primary object.

**Why.** Weakening constructor assertions breaks class invariants and leaves
objects in invalid, half-initialized states that risk null-pointer dereferences
or undefined behavior if other methods are invoked. Extracting self-contained
logic into dedicated helpers cleanly decouples calculation routines from object
lifecycles while preserving invariant enforcement.

## Examples

Bad:

```cpp
// Removing an assertion to instantiate a dummy SkWorkingFormatColorFilter with nullptr child
SkWorkingFormatColorFilter::SkWorkingFormatColorFilter(sk_sp<SkColorFilter> child, ...) {
    // SkASSERT(child); removed to allow external callers to invoke workingFormat()
    fChild = std::move(child);
    ...
}
```

Good:

```cpp
// Extract the calculation logic into an independent helper class
class SkWorkingFormatCalculator {
public:
    SkWorkingFormatCalculator(const skcms_TransferFunction* tf, const skcms_Matrix3x3* gamut, const SkAlphaType* at);
    sk_sp<SkColorSpace> workingFormat(const sk_sp<SkColorSpace>& dstCS, SkAlphaType* outAT) const;
};

SkWorkingFormatColorFilter::SkWorkingFormatColorFilter(sk_sp<SkColorFilter> child, ...)
        : fWorkingFormatCalculator(tf, gamut, at) {
    SkASSERT(child);
    fChild = std::move(child);
}
```

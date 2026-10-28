---
name: skia/use-limit-constants-for-bounds-checks
description: When declaring fixed arrays, bounding loops, clamping values, calculating offsets, or mirroring limits, use canonical constants instead of magic numbers and verify with static_assert
rubric: readability
severity: should_fix
---

**Rule.** Use canonical configuration, class, or struct limit constants (such as
Position::kMaxOffset, SparseStripConfig::kMaxTexturePages, or
GainCurve::kMaxNumControlPoints) instead of hardcoded magic numbers when
declaring fixed-size arrays, bounding loops, clamping ranges, computing
synthetic offsets or bounds, and asserting container sizes. When validating
sizes, clamping values, or allocating fixed-capacity buffers, reference the
existing named constants defined on the type or configuration struct.
Additionally, when defining internal or parsing constants that mirror public API
limits or related data structures, enforce synchronization with static_assert at
compile time.

**Why.** Hardcoded numeric literals obscure semantic intent and silently fall
out of sync when container limits, buffer capacities, or configuration constants
change. Referencing canonical limit constants keeps array dimensions, loop
iterations, clamping bounds, offsets, and assertions synchronized across
subsystems, while static_assert guarantees that mirrored internal and public
constants cannot silently diverge when either definition is modified.

## Examples

Bad:

```cpp
// Bad: hardcoded magic numbers in array bounds, loops, offsets, and bounds checks
TexturePage fPages[2];
for (int i = 0; i < 2; ++i) {
    fPages[i].reset();
}
fNumAlternateImages = clamp(syntax.num_alternate_images, 0, 4);
SkASSERT(cp.size() > 0 && cp.size() <= 32);
int syntheticOffset = 0x800000 - fInlinedFunctionCallCounter;

// Bad: duplicate internal constants without compile-time verification
struct AgtmSyntax {
    static constexpr uint8_t kMaxNumAlternateImages = 4;
    static constexpr uint8_t kMaxNumControlPoints = 32;
};
```

Good:

```cpp
// Good: reference canonical configuration and limit constants
TexturePage fPages[SparseStripConfig::kMaxTexturePages];
for (int i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
    fPages[i].reset();
}
fNumAlternateImages = clamp(syntax.num_alternate_images, 0u, kMaxNumAlternateImages);
SkASSERT_RELEASE(cp.size() >= AdaptiveGlobalToneMap::GainCurve::kMinNumControlPoints &&
                 cp.size() <= AdaptiveGlobalToneMap::GainCurve::kMaxNumControlPoints);
int syntheticOffset = Position::kMaxOffset - fInlinedFunctionCallCounter;

// Good: verify mirrored internal constants match public definitions
struct AgtmSyntax {
    static constexpr uint8_t kMaxNumAlternateImages = 4;
    static constexpr uint8_t kMaxNumControlPoints = 32;
};
static_assert(AgtmSyntax::kMaxNumAlternateImages ==
              skhdr::AdaptiveGlobalToneMap::HeadroomAdaptiveToneMap::kMaxNumAlternateImages);
static_assert(AgtmSyntax::kMaxNumControlPoints ==
              skhdr::AdaptiveGlobalToneMap::GainCurve::kMaxNumControlPoints);
```

---
name: skia/name-boolean-parameters-by-condition-not-side-effect
description: When declaring variables, parameters, or counters, name them after the concrete condition, property, or check rather than downstream actions or assumed states
rubric: readability
severity: should_fix
---

**Rule.** Name boolean variables, parameters, member flags, and tracking
counters after the underlying concrete condition, property, or check they
directly represent (e.g., isFirstElement, paramIsSpecialized, checkPinners,
fPinnerCount) rather than an assumed downstream action, side effect, or inferred
high-level state (e.g., clearMask, ignoreParam, includePinned,
fUnpinnedCacheCount). Keep the identifier focused on the tested state or
concrete attribute, especially when objects dynamically alter state or when
subsequent branches and assertions depend on that condition.

**Why.** Naming variables or counters after presumed actions, side effects, or
inferred states obscures intent and becomes misleading when the condition does
not unconditionally dictate that outcome (e.g., objects with a pinner may still
be dynamically deletable via canDelete(), or a flag only enables querying a
delegate). Describing the concrete property or check directly makes internal
control flow, state tracking, and caller call sites self-documenting and
resilient to code evolution.

## Examples

Bad:

```cpp
// Bad: 'clearMask', 'ignoreParam', and 'includePinned' name downstream actions
// or imply unconditional outcomes rather than describing the actual condition:
void draw_to_sw_mask(RasterMaskHelper* helper,
                     const ClipStack::Element& e,
                     bool clearMask,
                     const SkIRect& resultBounds);

bool ignoreParam = specializedParams && specializedParams->find(param);

// Misleadingly implies pinned resources are unconditionally included/purged:
size_t internalPurge(size_t minBytesNeeded = 0, bool includePinned = false);

// Misleading: counts fPinner == nullptr as 'unpinned', though pinned items may be deletable:
fUnpinnedCacheCount -= strike->fPinner == nullptr ? 1 : 0;
```

Good:

```cpp
// Good: names reflect the semantic condition, state, or conditional query performed:
void draw_to_sw_mask(RasterMaskHelper* helper,
                     const ClipStack::Element& e,
                     bool isFirstElement,
                     const SkIRect& resultBounds);

bool paramIsSpecialized = specializedParams && specializedParams->find(param);

// Clear: reflects that pinners will be queried (e.g. pinner->canDelete()) for eligibility:
size_t internalPurge(size_t minBytesNeeded = 0, bool checkPinners = false);

// Clear: directly tracks the concrete attribute checked (presence of fPinner):
fPinnerCount -= strike->fPinner != nullptr ? 1 : 0;
```

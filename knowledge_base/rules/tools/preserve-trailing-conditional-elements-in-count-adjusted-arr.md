---
name: skia/tools/preserve-trailing-conditional-elements-in-count-adjusted-arr
description: When adding elements to an array whose count is decremented to conditionally drop the trailing item, insert new items before the conditional element
rubric: correctness
severity: should_fix
---

**Rule.** Do not append new elements to the end of an array when the trailing
element is conditionally excluded by decrementing the count (such as
std::size(array) - (flag ? 0 : 1)). Always insert new permanent elements before
the conditionally dropped item, and document the trailing ordering invariant
with an explanatory comment.

**Why.** Decrementing an array count conditionally excludes whichever element
resides at the end of the array. Appending permanent elements after the
conditional item causes the newly added element to be conditionally dropped
instead while leaving the intended conditional element permanently active.

## Examples

Bad:

```cpp
static constexpr const char* kToggles[] = {
    "disable_robustness",
    // Must be last to correctly respond to `useTintIR` parameter.
    "use_tint_ir",
    "disable_lazy_clear_for_mapped_at_creation_buffer",
};
togglesDesc.enabledToggleCount = std::size(kToggles) - (useTintIR ? 0 : 1);
```

Good:

```cpp
static constexpr const char* kToggles[] = {
    "disable_robustness",
    "disable_lazy_clear_for_mapped_at_creation_buffer",
    // Must be last to correctly respond to `useTintIR` parameter.
    "use_tint_ir",
};
togglesDesc.enabledToggleCount = std::size(kToggles) - (useTintIR ? 0 : 1);
```

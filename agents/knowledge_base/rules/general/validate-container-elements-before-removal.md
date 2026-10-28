---
name: skia/validate-container-elements-before-removal
description: When validating or inspecting container elements being removed, perform assertions and reads before calling mutating removal methods like removeShuffle or erase
rubric: safety
severity: should_fix
---

**Rule.** Validate, inspect, or assert preconditions on container elements
before invoking mutating removal methods (such as removeShuffle(), erase(), or
pop_back()), rather than after the element has been removed or destroyed.
Calling mutating removal methods immediately destructs the element or swaps
another element into its slot, rendering any existing reference, pointer, or
iterator dangling or aliased to a different element.

**Why.** Mutating container operations like removeShuffle() destruct the element
at the target index in place and move the container's last element into that
slot. Reading or validating an element reference, pointer, or iterator after the
removal causes a use-after-free, reads invalid memory, or inadvertently tests
the swapped element rather than the intended target.

## Examples

Bad:

```cpp
const MaskHashEntry& entry = (*cachedArray)[i];
if (entry.fBounds == currEntry->fBounds) {
    cachedArray->removeShuffle(i);
    // Dangling reference: entry was destructed or replaced by the swapped element.
    SkASSERT_RELEASE(index == fDrawAtlas->getListIndex(entry.fLocator.plotLocator()));
    break;
}
```

Good:

```cpp
const MaskHashEntry& entry = (*cachedArray)[i];
if (entry.fBounds == currEntry->fBounds) {
    // Validate entry properties before mutating removal invalidates the reference.
    SkASSERT_RELEASE(index == fDrawAtlas->getListIndex(entry.fLocator.plotLocator()));
    cachedArray->removeShuffle(i);
    break;
}
```

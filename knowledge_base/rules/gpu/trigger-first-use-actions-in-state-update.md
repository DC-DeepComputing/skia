---
name: skia/gpu/trigger-first-use-actions-in-state-update
description: When stateful recording elements detect initial usage via existing state, trigger first-use actions inside the state update method instead of checking caller-side boolean flags
rubric: design
severity: should_fix
---

**Rule.** When an element or recording helper already detects initial usage
through internal state transitions (such as uninitialized order or empty usage
bounds), trigger first-use actions and side effects directly within that state
update method. Do not introduce separate boolean flags (such as fDrawn) to track
initial usage and dispatch first-use actions conditionally at caller sites.

**Why.** Encapsulating first-use side effects within the state update method
keeps lifecycle transitions localized and self-contained. Exposing and checking
separate boolean flags at caller sites duplicates lifecycle checks, leaks
internal state management, and requires redundant flag reset and synchronization
boilerplate across flush cycles.

## Examples

Bad:

```cpp
// Caller checks a separate boolean flag to trigger first-use actions
order = element->updateForDraw(boundsManager, deviceBounds, clipBounds, z);
if (useDrawListLayer && !element->hasBeenDrawn()) {
    prevLayer = element->drawClipImmediate(device, prevLayer);
}
```

Good:

```cpp
// First-use action is triggered internally where first usage is already detected
order = element->updateForDraw(device, boundsManager, deviceBounds, clipBounds, z, prevLayer);

// Inside updateForDraw():
if (!this->hasPendingDraw()) {
    if (device->caps()->useDrawListLayer()) {
        deferredLayer = this->drawClipImmediate(device, deferredLayer);
    }
}
```

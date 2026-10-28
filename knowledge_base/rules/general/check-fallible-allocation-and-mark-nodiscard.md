---
name: skia/check-fallible-allocation-and-mark-nodiscard
description: When defining or calling fallible resource-allocation and initialization functions, annotate signatures with [[nodiscard]] and verify callers check return status before accessing resources
rubric: correctness
severity: should_fix
---

**Rule.** When defining fallible resource-allocation, activation, or
initialization functions, annotate their declarations with [[nodiscard]] to
prevent callers from silently ignoring failures. Callers must check the boolean
status or returned handle before accessing newly allocated resources, handling
errors gracefully or returning early rather than assuming allocation succeeded.

**Why.** Resource allocation and activation operations (such as GPU texture
creation, atlas page activation, or buffer allocation) can fail under memory
pressure or unsupported configurations. Ignoring return values leads to null
pointer dereferences or crashes when accessing unallocated resources;
[[nodiscard]] catches unhandled failures at compile time.

## Examples

Bad:

```cpp
// Missing [[nodiscard]] on fallible allocation or activation method:
bool activateNewPage(Recorder* recorder);

// Caller ignores return value and dereferences unallocated resource:
if (fNumActivePages == 0) {
    this->activateNewPage(recorder);
}
const Plot* plot = fPages[0].fMRUList.head();
```

Good:

```cpp
// Enforce failure handling at compile time:
[[nodiscard]] bool activateNewPage(Recorder* recorder);

// Caller checks return status before accessing resource:
if (fNumActivePages == 0) {
    if (!this->activateNewPage(recorder)) {
        return ErrorCode::kError;
    }
}
const Plot* plot = fPages[0].fMRUList.head();
```

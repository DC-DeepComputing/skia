---
name: skia/api/preserve-container-element-lifecycle-before-shrink
description: When resizing or trimming custom containers like skia_private::TArray, do not manipulate internal size tracking to invoke shrink_to_fit without running element constructors and destructors
rubric: safety
severity: should_fix
---

**Rule.** In custom container implementations (such as skia_private::TArray), do
not mutate internal element count or size tracking (e.g. via changeSize()) to
force buffer reallocation via shrink_to_fit(). Always ensure container size
invariants match the actual range of constructed elements so relocation and
destruction do not operate on uninitialized memory or leak existing instances.

**Why.** Container shrinking and relocation methods like shrink_to_fit() invoke
move constructors and destructors assuming that [0, fSize) contains initialized
objects. Artificially modifying size tracking causes relocation logic to move or
destroy uninitialized memory and skips destruction of valid elements when
reducing capacity.

## Examples

Bad:

```cpp
void shrink_and_clear(int count) {
    // Dangerous: fSize no longer reflects initialized objects
    this->changeSize(count);
    this->shrink_to_fit();
    this->changeSize(0);
}
```

Good:

```cpp
// Respect element lifecycle and use dedicated capacity methods on empty containers
SkASSERT(this->empty());
this->reserve_exact(targetCapacity);
```

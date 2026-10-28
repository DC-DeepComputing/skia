---
name: skia/omit-redundant-loop-guards
description: When iterating over collections or ranges, omit redundant size, empty, or boundary checks before a loop whose condition already safely handles those edge cases
rubric: readability
severity: nit
---

**Rule.** Do not add redundant size, emptiness, or boundary checks (such as if
(len <= 1) or if (container.empty()) return;) before a loop when the loop
condition already handles those boundary cases without executing the body. Rely
on the loop condition naturally evaluating to false on empty or single-element
inputs to fall through to the expected return value.

**Why.** Early-exit guards that duplicate the loop condition's termination
behavior add redundant branching and dead code. Omitting unnecessary guards
simplifies control flow and allows standard loop semantics to handle boundary
cases cleanly.

## Examples

Bad:

```cpp
static bool all_pixels_same_color(uint32_t* buffer, size_t len) {
    if (len <= 1) {
        return true;
    }
    for (size_t i = 1; i < len; ++i) {
        if (buffer[0] != buffer[i]) {
            return false;
        }
    }
    return true;
}
```

Good:

```cpp
static bool all_pixels_same_color(uint32_t* buffer, size_t len) {
    for (size_t i = 1; i < len; ++i) {
        if (buffer[0] != buffer[i]) {
            return false;
        }
    }
    return true;
}
```

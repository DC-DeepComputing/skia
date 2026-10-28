---
name: skia/guard-empty-container-before-size-minus-one
description: When computing container upper bounds with size() - 1, check empty() first instead of relying on unsigned comparisons like end <= 0
rubric: correctness
severity: blocking
---

**Rule.** Do not rely on unsigned comparisons like container.size() - 1 <= 0 or
end <= start to guard empty containers; explicitly check container.empty() (or
container.size() > 0) before computing or indexing container bounds. Container
size() returns an unsigned type that wraps around to SIZE_MAX on empty
containers, causing non-positive comparisons to evaluate to false and bypassing
bounds guards.

**Why.** In C++, container size() returns an unsigned integer type like size_t.
When a container is empty, subtracting one (size() - 1) underflows to SIZE_MAX
rather than a negative value. Comparisons checking whether the resulting index
is non-positive or less than a starting index evaluate to false, allowing
execution to proceed and trigger out-of-bounds container access.

## Examples

Bad:

```cpp
size_t startLine = 0;
size_t endLine = fLines.size() - 1;
// Bug: if fLines is empty, endLine underflows to SIZE_MAX and endLine <= startLine is false!
if (endLine <= startLine || fLines[endLine].textWithNewlines().end <= codeUnitIndex) {
    return -1;
}
```

Good:

```cpp
if (fLines.empty()) {
    return -1;
}
size_t startLine = 0;
size_t endLine = fLines.size() - 1;
if (fLines[endLine].textWithNewlines().end <= codeUnitIndex) {
    return -1;
}
```

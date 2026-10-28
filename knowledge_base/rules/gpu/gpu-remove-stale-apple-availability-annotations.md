---
name: skia/gpu/gpu-remove-stale-apple-availability-annotations
description: When writing or refactoring Apple backend code, remove compile-time SK_API_AVAILABLE annotations and runtime @available checks satisfied by Skia's baseline OS versions
rubric: readability
severity: should_fix
---

**Rule.** Do not retain or introduce compile-time SK_API_AVAILABLE(...)
annotations or runtime @available checks for Apple operating system versions
that are unconditionally satisfied by Skia's minimum deployment targets (macOS
11+, iOS 13+). When cleaning up runtime @available blocks or implementing Apple
backend APIs, remove obsolete availability annotations targeting earlier OS
versions (such as macOS 10.11 or iOS 8/9/12).

**Why.** Skia's minimum supported deployment baseline for Apple platforms is
macOS 11 and iOS 13. Guarding methods or calls against earlier OS releases is
dead code that adds visual clutter and misleadingly implies older platform
versions are supported.

## Examples

Bad:

```cpp
void setBufferOffset(NSUInteger offset, NSUInteger index)
        SK_API_AVAILABLE(macos(10.11), ios(8.3), tvos(9.0)) {
    SkASSERT(index < kMaxExpectedBuffers);
    if (fBufferOffsets[index] != offset) {
        [(*fCommandEncoder) setBufferOffset:offset atIndex:index];
        fBufferOffsets[index] = offset;
    }
}
```

Good:

```cpp
void setBufferOffset(NSUInteger offset, NSUInteger index) {
    SkASSERT(index < kMaxExpectedBuffers);
    if (fBufferOffsets[index] != offset) {
        [(*fCommandEncoder) setBufferOffset:offset atIndex:index];
        fBufferOffsets[index] = offset;
    }
}
```

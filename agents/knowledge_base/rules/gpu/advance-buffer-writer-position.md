---
name: skia/gpu/advance-buffer-writer-position
description: When implementing writing, clearing, or padding methods on sequential buffer writers, advance the writer's position after modifying the underlying buffer
rubric: correctness
severity: should_fix
---

**Rule.** When implementing buffer writing, clearing, or padding methods on
sequential buffer writers (such as BufferWriter, VertexWriter, or IndexWriter),
advance the writer's offset or slice the required range (e.g., via
this->slice(bytes) or *this = this->makeOffset(bytes)) after modifying the
buffer. Never leave the internal buffer pointer unadjusted after writing or
zeroing bytes.

**Why.** Failing to advance the buffer writer's position after writing or
clearing bytes leaves the current pointer and bounds marker unchanged.
Subsequent writes will overwrite the newly padded or cleared memory, corrupting
data and invalidating internal bounds tracking.

## Examples

Bad:

```cpp
void zeroBytes(size_t bytesToZero) {
    this->validate(bytesToZero);
    memset(fPtr, 0, bytesToZero);
}
```

Good:

```cpp
void zeroBytes(size_t bytesToZero) {
    this->validate(bytesToZero);
    memset(fPtr, 0, bytesToZero);
    *this = this->makeOffset(bytesToZero);
}
```

---
name: skia/gpu/reset-staging-buffers-on-finalization
description: When finalizing GPU buffer data from CPU staging storage during recording, return boolean failure on buffer writer allocation failure and reset CPU staging memory upon success
rubric: correctness
severity: should_fix
---

**Rule.** When defining finalization methods that commit CPU staging data into
GPU buffers (such as via DrawBufferManager writers), return a boolean status or
std::optional and verify buffer writer acquisition succeeded before writing.
Propagate allocation failure to abort recording or draw-pass snapping, and reset
CPU staging containers and deduplication caches upon successful write.

**Why.** Buffer allocation and memory mapping in DrawBufferManager can fail
under memory exhaustion or unsupported configurations, returning an invalid
writer. Ignoring acquisition failures leads to null-dereference crashes or
corrupted recordings, while failing to reset CPU staging storage retains
unnecessary host memory across subsequent render passes.

## Examples

Bad:

```cpp
// BAD: Ignores buffer writer acquisition failure and retains CPU staging storage
void finalize(DrawBufferManager* bufferMgr) {
    if (!fGradientStorage.empty()) {
        auto [writer, bufferInfo] = bufferMgr->getSsboWriter(fGradientStorage.size(),
                                                             sizeof(float));
        if (writer) {
            writer.write(fGradientStorage.data(), fGradientStorage.size_bytes());
            fBufferInfo = bufferInfo;
        }
    }
}
```

Good:

```cpp
// GOOD: Propagates writer failure to fail recording and resets CPU storage on success
bool finalize(DrawBufferManager* bufferMgr) {
    if (!fGradientStorage.empty()) {
        auto [writer, bufferInfo] = bufferMgr->getSsboWriter(fGradientStorage.size(),
                                                             sizeof(float));
        if (writer) {
            writer.write(fGradientStorage.data(), fGradientStorage.size_bytes());
            fBufferInfo = std::optional(bufferInfo);
            this->reset();
        } else {
            return false;
        }
    }
    return true;
}
```

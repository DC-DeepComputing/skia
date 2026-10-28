---
name: skia/gpu/document-flush-before-paint-key-order
description: When sequencing pre-draw flushes during Graphite draw recording, flush pending work before evaluating paint keys and document that notifyInUse registers child tasks
rubric: readability
severity: should_fix
---

**Rule.** When sequencing pre-draw flushes during Graphite draw recording (such
as in Device::drawGeometry), flush pending work before evaluating paint keys
(shading.toKey()). Additionally, document this ordering invariant in an inline
comment: paint key generation invokes notifyInUse() on referenced textures or
devices, so flushing beforehand ensures child tasks attach to the current draw
rather than flushed predecessor tasks.

**Why.** Generating shader and paint keys can trigger notifyInUse() callbacks
that register dependent child tasks into the active DrawContext. Flushing
pending work after key generation finalizes those child tasks alongside
predecessor tasks instead of sequencing them before the current draw;
documenting this invariant prevents maintainers from reordering key generation
ahead of flushes.

## Examples

Bad:

```cpp
// Decide if we have any reason to flush pending work before updating clips or path atlases.
const bool needsFlush = this->needsFlushBeforeDraw(numNewRenderSteps, dstReadStrategy);
if (needsFlush) {
    this->flushPendingWorkToRecorder();
}
// Bad: Omits documentation of why flush must precede shading.toKey().
auto keyResult = shading.toKey(keyContext);
```

Good:

```cpp
// Decide if we have any reason to flush pending work. A flush is required:
// 1) Before updating clip state or atlas entries to prevent flushing current draw state.
// 2) Before shading.toKey() so child tasks from notifyInUse() attach to this draw
//    instead of being finalized with predecessor tasks.
const bool needsFlush = this->needsFlushBeforeDraw(numNewRenderSteps, dstReadStrategy);
if (needsFlush) {
    this->flushPendingWorkToRecorder();
}
auto keyResult = shading.toKey(keyContext);
```

---
name: skia/gpu/assert-template-invariants-while-pending
description: When updating template or static buffers in draw batchers like DrawWriter, assert buffer invariance while draws are pending instead of defensively flushing on mismatch
rubric: correctness
severity: should_fix
---

**Rule.** In draw-recording and batching helpers (such as DrawWriter), assert
with SkASSERT that template buffers (including static data, vertices, instances,
and indices) and counts remain invariant while draws are pending (fPendingCount
!= 0), and only assign new template or static state when fPendingCount == 0. Do
not defensively inspect template mismatches or invoke flush() inside template
setters, nor accept mismatched buffers without verifying they match
already-bound state.

**Why.** Within a single pipeline state, render steps should not switch template
buffers without an explicit pipeline change or dynamic state transition that
already triggers a flush. Defensively checking for template mismatches and
flushing hides unexpected state transitions, while accepting mismatched buffers
without asserting invariance risks executing draw commands against stale or
incorrect GPU buffer bindings.

## Examples

Bad:

```cpp
// Defensively checking for template mismatches and flushing inside template setter:
bool willBind = vertices != fVertices || instances != fInstances || indices != fIndices;
if (fPendingCount && willBind) {
    this->flush();
}
fVertices = vertices;

// Or accepting new static/template buffers while draws are pending without checking:
void DrawWriter::setTemplate(BindBufferInfo staticData, BindBufferInfo indices, uint32_t count) {
    fStatic = staticData;
    fIndices = indices;
}
```

Good:

```cpp
// Assign template state only when clean, and assert invariance when draws are pending:
void DrawWriter::setTemplate(BindBufferInfo staticData, BindBufferInfo indices, uint32_t count) {
    if (fPendingCount == 0) {
        fStatic = staticData;
        fIndices = indices;
        fTemplateCount = count;
    } else {
        SkASSERT(fStatic == staticData && fIndices == indices);
        SkASSERT((count == 0 && (fRenderState & RenderStateFlags::kAppendDynamicInstances)) ||
                 fTemplateCount == count);
    }
}
```

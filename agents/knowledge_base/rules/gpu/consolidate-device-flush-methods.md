---
name: skia/gpu/consolidate-device-flush-methods
description: When implementing Graphite Device task flushing, consolidate destination flush methods into an optional DrawContext helper and reset draw-pass state in internalFlush
rubric: design
severity: should_fix
---

**Rule.** When implementing task flushing in Graphite (such as in Device),
consolidate duplicate flush methods targeting different destinations into a
single helper accepting an optional DrawContext* parameter. When a destination
DrawContext is provided, record the snapped task as a dependency without
modifying scratch device tracking; when nullptr, append the task to the root
task list and update fLastTask for scratch devices. Additionally, place common
draw-pass state resets—such as clearing fPriorDrawDstReadStrategy—and pass
finalization inside internalFlush() rather than duplicating cleanup across outer
flush entry points.

**Why.** Consolidating outer flush entry points avoids duplicating task-snapping
and lifecycle branching while ensuring fLastTask updates remain restricted to
root flushes. Placing pass finalization and draw-pass state resets in
internalFlush() ensures all flush pathways consistently reset device tracking
state without duplicating cleanup.

## Examples

Bad:

```cpp
void Device::flushToDrawContext(DrawContext* drawContext) {
    this->internalFlush();
    sk_sp<Task> drawTask = fDC->snapDrawTask(fRecorder);
    drawContext->recordDependency(std::move(drawTask));
    // Duplicated state cleanup across outer flush functions
    fPriorDrawDstReadStrategy = DstReadStrategy::kNoneRequired;
    fIsFlushing = false;
}

void Device::flushPendingWorkToRecorder() {
    this->internalFlush();
    sk_sp<Task> drawTask = fDC->snapDrawTask(fRecorder);
    if (this->isScratchDevice()) {
        fLastTask = drawTask;
    }
    if (drawTask) {
        fRecorder->priv().add(std::move(drawTask));
    }
    fPriorDrawDstReadStrategy = DstReadStrategy::kNoneRequired;
    fIsFlushing = false;
}
```

Good:

```cpp
void Device::internalFlush() {
    fRecorder->priv().atlasProvider()->recordUploads(fDC.get());
    fClip.recordDeferredClipDraws();
    fRecorder->priv().atlasProvider()->compact();
    // Common pass cleanup lives in internalFlush()
    fPriorDrawDstReadStrategy = DstReadStrategy::kNoneRequired;
}

void Device::flushPendingWork(DrawContext* drawContext) {
    this->internalFlush();
    sk_sp<Task> drawTask = fDC->snapDrawTask(fRecorder);
    if (drawContext) {
        drawContext->recordDependency(std::move(drawTask));
    } else {
        fLastTask = this->isScratchDevice() ? drawTask : nullptr;
        if (drawTask) {
            fRecorder->priv().add(std::move(drawTask));
        }
    }
    fIsFlushing = false;
}
```

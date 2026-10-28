---
name: skia/gpu/call-notify-in-use-on-graphite-image-sampling
description: When routing Graphite tasks, sampling images, or declaring optional DrawContext helpers, sequence tasks via DrawContext and document flush operations, task destinations, or intentional nullptr
rubric: correctness
severity: should_fix
---

**Rule.** When extracting a TextureProxy directly from an Image outside standard
draw paths, invoke notifyInUse(recorder, drawContext) to link prerequisite
tasks. When generating intermediate tasks or calling subroutines like
GenerateMipmaps, forward the active DrawContext* to route tasks via
drawContext->recordDependency() rather than passing nullptr or adding to
recorder->priv().add(). When intentionally passing nullptr DrawContext, document
that root-list placement is intentional. Additionally, when declaring helpers or
flush methods accepting an optional DrawContext*, document in header comments
the underlying operations and side effects (such as resolving clips or snapping
RenderPassTasks) and whether tasks are added as DrawContext dependencies or
enqueued to the root task list.

**Why.** In Graphite, tasks added directly to the Recorder root list execute
independently rather than sequencing before dependent draws in an active
DrawContext. Forwarding DrawContext to helpers like GenerateMipmaps ensures mip
generation tasks sequence properly with dependent draws, while documenting flush
side effects and intentional nullptr parameters prevents speculative TODOs or
invalid task sequencing.

## Examples

Bad:

```cpp
// 1. Sampling image proxy directly without notifyInUse:
dstCopy = drawnImage->textureProxyView().refProxy();

// 2. Passing nullptr to subroutines or adding tasks to Recorder directly:
GenerateMipmaps(recorder, /*drawContext=*/nullptr, dst, srcColorInfo);
recorder->priv().add(std::move(copyTask));

// 3. Speculative TODO when passing nullptr DrawContext:
// TODO: Could this be moved onto the drawContext list?
return Image::Copy(fRecorder, /*drawContext=*/nullptr, ...);

// 4. Undocumented flush side effects and task destination for DrawContext*:
// Flushes pending work to draw context.
void flushPendingWork(DrawContext*);
```

Good:

```cpp
// 1. Register image usage on the active DrawContext:
static_cast<Image_Base*>(drawnImage.get())->notifyInUse(recorder, drawContext);
dstCopy = drawnImage->textureProxyView().refProxy();

// 2. Forward DrawContext to subroutines and route tasks via recordDependency:
GenerateMipmaps(recorder, drawContext, dst, srcColorInfo);
if (drawContext) {
    drawContext->recordDependency(std::move(copyTask));
} else {
    recorder->priv().add(std::move(copyTask));
}

// 3. Document intentional nullptr DrawContext for root task list placement:
// Passing nullptr ensures tasks end up on the root task list for the container to consume.
return Image::Copy(fRecorder, /*drawContext=*/nullptr, ...);

// 4. Document underlying flush operations and task routing:
// Snaps pending work as RenderPassTask; if drawContext is non-null, flushes to its
// task list, else adds to root list and records lastTask on scratch devices.
void flushPendingWork(DrawContext*);
```

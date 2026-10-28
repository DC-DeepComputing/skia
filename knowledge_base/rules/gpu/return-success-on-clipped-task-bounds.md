---
name: skia/gpu/return-success-on-clipped-task-bounds
description: When intersecting replay clips or task bounds with render targets in Graphite tasks, verify the intersection and return Status::kSuccess early without emitting commands
rubric: correctness
severity: should_fix
---

**Rule.** In Graphite tasks (such as RenderPassTask or UploadTask), check the
boolean return value when intersecting replay clips, draw bounds, or copy rects
with target dimensions. If the bounds do not intersect or are clipped out,
return Status::kSuccess early without recording commands to the command buffer
rather than returning Status::kFail or emitting unclipped commands.

**Why.** In Graphite's task execution model, conditionally skipped or
clipped-out work must return Status::kSuccess so the recording remains valid and
subsequent tasks continue replaying, whereas Status::kFail invalidates the
entire recording. Furthermore, ignoring intersect() results risks emitting
redundant GPU commands or incorrectly treating an empty clip rectangle as
unclipped.

## Examples

Bad:

```cpp
SkIRect replayClip = replayData.fClip;
// Missing intersection check can emit unnecessary GPU commands or treat empty clip as unclipped
replayClip.intersect(colorAttachmentBounds);
commandBuffer->setReplayTranslationAndClip(replayData.fTranslation, replayClip);
```

Good:

```cpp
SkIRect replayClip = replayData.fClip;
if (!replayClip.intersect(colorAttachmentBounds)) {
    return Status::kSuccess;
}
commandBuffer->setReplayTranslationAndClip(replayData.fTranslation, replayClip);
```

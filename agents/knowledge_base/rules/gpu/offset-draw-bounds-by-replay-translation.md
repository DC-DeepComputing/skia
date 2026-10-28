---
name: skia/gpu/offset-draw-bounds-by-replay-translation
description: When replaying Graphite tasks against translated or scratch targets, offset local bounds by replay translation and express render pass bounds in active attachment coordinates
rubric: design
severity: should_fix
---

**Rule.** When Graphite tasks process recorded draw or copy bounds against a
replay target, offset recording-local bounds by ReplayTargetData::fTranslation
before intersecting with target dimensions or computing attachment and resolve
offsets. Additionally, express render pass bounds, viewports, and replay
translations in the coordinate space of the active color/MSAA attachment (e.g.,
replayTranslation - resolveOffset and
SkIRect::MakeSize(colorAttachment->dimensions())) rather than resolve target
coordinates, avoiding threading ad-hoc offsets into CommandBuffer draw passes
and backend scissor methods.

**Why.** Testing unadjusted local bounds against target dimensions clips
geometry incorrectly and miscalculates resolve offsets. Defining render pass
bounds and replay translations in active attachment coordinates allows
CommandBuffer backend draw pass and scissor commands to run unmodified in
attachment space, eliminating ad-hoc coordinate shifting across backend
subclasses.

## Examples

Bad:

```cpp
// 1. Unadjusted drawBounds passed to resolve calculation:
std::tie(msaaSize, resolveOffset) = get_msaa_size_and_resolve_offset(
        fTarget->dimensions(), drawBounds, *caps, fLoadOp);

// 2. Render pass bounds kept in resolve texture space, forcing subclasses to offset scissors:
commandBuffer->setReplayTranslationAndClip(
        replayTranslation, replayClip, SkIRect::MakeSize(fTarget->dimensions()));
...
void DawnCommandBuffer::setScissor(const Scissor& scissor,
                                   const SkIPoint& viewportTranslation) {
    SkIRect rect = scissor.getRect(fReplayTranslation, fRenderPassBounds);
    rect.offset(viewportTranslation);
    fActiveRenderPassEncoder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
}
```

Good:

```cpp
// 1. Offset drawBounds by replayTranslation before computing resolve extents:
SkIVector replayTranslation = fTarget->texture() == replayData.fTarget ? replayData.fTranslation : SkIVector{0, 0};
std::tie(msaaSize, resolveOffset) = get_msaa_size_and_resolve_offset(
        fTarget->dimensions(), drawBounds.makeOffset(replayTranslation), *caps, fLoadOp);

// 2. Express renderPassBounds in color attachment space and embed resolveOffset:
commandBuffer->setReplayTranslationAndClip(
        replayTranslation - resolveOffset, replayClip, SkIRect::MakeSize(colorAttachment->dimensions()));
...
void DawnCommandBuffer::setScissor(const Scissor& scissor) {
    SkIRect rect = scissor.getRect(fReplayTranslation, fRenderPassBounds);
    fActiveRenderPassEncoder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
}
```

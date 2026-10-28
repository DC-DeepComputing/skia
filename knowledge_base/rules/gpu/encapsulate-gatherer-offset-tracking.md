---
name: skia/gpu/encapsulate-gatherer-offset-tracking
description: When designing multi-phase gatherers or accumulators in Graphite, encapsulate phase-boundary checkpointing across all resources and track active managers and starting offsets internally
rubric: design
severity: should_fix
---

**Rule.** When managing multi-phase data collection in coordinating gatherers or
accumulators (such as PipelineDataGatherer and UniformManager), encapsulate
phase state and sub-range tracking internally. In gatherers, expose
phase-agnostic write methods and consolidate phase-boundary checkpointing for
all managed resources (such as uniform offsets and texture counts) into
lifecycle transition methods (e.g., markOffsetAndAlign() or endPaintData())
rather than requiring callers to invoke separate per-resource marking or rewind
calls. In buffer accumulators, track starting offsets internally so wrapper
objects (such as UniformDataBlock::Wrap) encapsulate subspan slicing without
requiring callers to slice raw buffers.

**Why.** Exposing phase-specialized write methods or requiring callers to
coordinate separate mark, rewind, or resource-tracking calls leaks internal
phase state and risks desynchronization across lifecycle stages. Furthermore,
forcing callers to slice raw buffer subspans bypasses encapsulation and requires
fragile friend declarations, whereas internalizing start offsets ensures correct
relative alignment and keeps wrapper construction uniform.

## Examples

Bad:

```cpp
// 1. Exposing phase-specialized write methods churns callers:
template <typename T> void writePaint(const T& t) { fPaintUniformManager.write(t); }
gatherer->writePaint(transform.matrix());

// 2. Exposing separate marking methods for different resources at the same phase boundary:
gatherer->markTextureSize();
gatherer->markOffsetAndAlign(performsShading, alignment);

// 3. Caller manually slices accumulator storage to construct wrapper objects:
SkSpan<const char> renderStepData = fUniformManager.finish().subspan(fPaintDataSize);
return {UniformDataBlock(renderStepData), textures}; // requires friend class
```

Good:

```cpp
// 1. Internal active manager tracking preserves a stable, phase-agnostic write interface:
template <typename T> void write(const T& t) { fActiveManager->write(t); }
gatherer->write(transform.matrix());

// 2. Single transition method consolidates checkpointing for all managed resources:
void markOffsetAndAlign(bool performsShading, int requiredAlignment) {
    fPaintTextureCount = fTextures.size();
    fUniformManager.markOffset();
    if (!performsShading) {
        fUniformManager.alignForNonShading(requiredAlignment);
    }
}

// 3. Accumulator tracks starting offset so wrappers encapsulate subspans without friend access:
return {UniformDataBlock::Wrap(&fUniformManager), textures};
```

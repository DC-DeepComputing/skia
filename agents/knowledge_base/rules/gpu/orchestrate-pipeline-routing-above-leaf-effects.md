---
name: skia/gpu/orchestrate-pipeline-routing-above-leaf-effects
description: When generating Graphite pipeline keys, do not pass contextual input origins to leaf effects or blenders; manage routing in higher-level orchestrators
rubric: design
severity: should_fix
---

**Rule.** When generating Graphite shader and pipeline keys, keep leaf shader,
color filter, and blender implementations decoupled from pipeline-level routing
decisions and contextual input origins (such as whether inputs originate from
the destination surface, primitive colors, or intermediate stages). Do not pass
contextual destination or source descriptors into leaf key-generation methods;
let higher-level orchestrating layers (such as PaintParams and KeyHelpers)
manage wrapper blocks, snippet hierarchy, and input routing.

**Why.** Leaf effect and blender objects should only be responsible for emitting
their own localized snippet and data. Coupling them to contextual knowledge
about input sources or delegating pipeline structure decisions violates
separation of concerns and prevents reusing the same leaf snippets across
different pipeline configurations.

## Examples

Bad:

```cpp
// Bad: Leaf blender takes contextual destination parameters and decides pipeline wrapping
void addToKey(const KeyContext& keyContext,
              PaintParamsKeyBuilder* builder,
              PipelineDataGatherer* gatherer,
              DstColorType dstColorType) const override {
    if (dstColorType == DstColorType::kSurface) {
        BlendShaderBlock::BeginBlock(keyContext, builder, gatherer);
        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer, {fEffect, uniforms});
    } else {
        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer, {fEffect, uniforms});
    }
}
```

Good:

```cpp
// Good: Leaf blender only emits its own snippet; orchestrators configure wrapper blocks
void addToKey(const KeyContext& keyContext,
              PaintParamsKeyBuilder* builder,
              PipelineDataGatherer* gatherer) const override {
    RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                   {fEffect, std::move(uniforms)});
    add_children_to_key(fChildren, fEffect->children(), keyContext, builder, gatherer);
    builder->endBlock();
}
```

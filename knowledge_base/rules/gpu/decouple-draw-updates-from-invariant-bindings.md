---
name: skia/gpu/decouple-draw-updates-from-invariant-bindings
description: When recording batched GPU draws sharing pipeline and texture bindings, decouple dynamic per-draw state updates from binding invariance checks
rubric: safety
severity: should_fix
---

**Rule.** When batching or recording grouped draw commands that share pipeline
and texture bindings but differ in per-draw dynamic data (such as uniforms),
decouple per-draw updates from binding invariance checks. Ensure subsequent
draws in the batch can treat shared bindings as invariant without skipping
necessary per-draw state updates.

**Why.** Coupling per-draw state updates (e.g., writing dynamic uniforms) with
binding invariance guards forces callers to either treat bindings as variant on
every draw—incurring redundant pipeline and texture binding checks—or skip
required per-draw data updates. Separating invariant state validation from
per-draw state updates allows subsequent draws in a batch to skip redundant
binding checks.

## Examples

Bad:

```cpp
// Bad: per-draw uniform updates are gated behind binding invariance, forcing callers to treat bindings as variant for every draw
if (!bindingsAreInvariant) {
    pipelineChange = key.fPipelineIndex != lastPipeline;
    uniformBindingChange = uniformTracker.writeUniforms(fUniformDataCache, bufferMgr, uniformIndex);
    textureBindingsChange = textureBindingTracker.setCurrentTextureBindings(key.fTextureIndex);
}
for (const SingleDraw* draw : singleList->fDraws) {
    recordDraw(binding->fKey, draw->fUniformIndex, binding->fStep, *draw->fDrawParams, false);
}
```

Good:

```cpp
// Good: decouple per-draw updates from invariant binding checks, passing bindingsAreInvariant = true for subsequent draws
if (!bindingsAreInvariant) {
    pipelineChange = key.fPipelineIndex != lastPipeline;
    textureBindingsChange = textureBindingTracker.setCurrentTextureBindings(key.fTextureIndex);
}
bool uniformBindingChange = uniformTracker.writeUniforms(fUniformDataCache, bufferMgr, uniformIndex);

const SingleDraw* current = singleList->fDraws.head();
recordDraw(binding->fKey, current->fUniformIndex, binding->fStep, *current->fDrawParams, false);
current = current->fNext;
while (current) {
    recordDraw(binding->fKey, current->fUniformIndex, binding->fStep, *current->fDrawParams, true);
    current = current->fNext;
}
```

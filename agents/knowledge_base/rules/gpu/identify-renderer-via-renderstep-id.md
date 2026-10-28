---
name: skia/gpu/identify-renderer-via-renderstep-id
description: When identifying a Graphite Renderer technique or step, compare its RenderStepID rather than pointer-comparing against RendererProvider singleton instances
rubric: readability
severity: should_fix
---

**Rule.** When identifying a Renderer technique or checking for a specific
rendering step in Graphite, inspect the step's RenderStepID (e.g.,
renderer->step(0).renderStepID() == RenderStep::RenderStepID::k...) rather than
comparing the Renderer* pointer against singleton instances retrieved from
RendererProvider. Do not couple local draw or device inspection logic to
Recorder or RendererProvider singletons when RenderStepID directly identifies
the technique.

**Why.** In Graphite, rendering techniques decompose into RenderSteps whose
RenderStepID enum values uniquely and directly identify the underlying
technique. Comparing against singleton instances fetched from RendererProvider
(such as via fRecorder->priv().rendererProvider()) introduces unnecessary
pointer indirections and couples local inspection code to the recorder and
provider.

## Examples

Bad:

```cpp
if (renderer == fRecorder->priv().rendererProvider()->sparseStripsEndCap()) {
    numNewRenderSteps +=
            fRecorder->priv().rendererProvider()->sparseStripsWideTile()->numRenderSteps();
}
```

Good:

```cpp
if (renderer->step(0).renderStepID() == RenderStep::RenderStepID::kEndCap) {
    numNewRenderSteps +=
            fRecorder->priv().rendererProvider()->sparseStripsWideTile()->numRenderSteps();
}
```

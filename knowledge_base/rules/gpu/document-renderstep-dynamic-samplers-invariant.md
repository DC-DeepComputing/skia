---
name: skia/gpu/document-renderstep-dynamic-samplers-invariant
description: When populating default SamplerDescs for Graphite RenderStep samplers, document the invariant that RenderSteps only use dynamic samplers and never immutable samplers
rubric: readability
severity: nit
---

**Rule.** When generating pipeline layouts or determining sampler requirements
for Graphite RenderSteps, explicitly document with an inline comment (or assert)
when code assumes RenderSteps only utilize dynamic samplers. Clarify that
RenderSteps never use immutable or static samplers when populating default or
uninitialized SamplerDescs.

**Why.** In Graphite, paint nodes may require immutable samplers (e.g., for
external or YCbCr textures) that must be identified with specific SamplerDescs
to bake sampler state into pipeline layouts, whereas RenderSteps only utilize
dynamic samplers (e.g., for coverage masks, atlases, or blurs). Documenting this
assumption clarifies that omitting immutable sampler lookups for RenderStep
samplers is an intentional design choice rather than an oversight.

## Examples

Bad:

```cpp
// Determine how many render step samplers were used by comparing the binding value
// against paintTextureCount, taking into account the binding requirements.
int renderStepSamplerCount = bindingReqs.fSeparateTextureAndSamplerBinding
        ? (binding - paintTextureCount) / 2
        : binding - paintTextureCount;
static constexpr SamplerDesc kUninitializedSamplerDesc {};
outDescs->push_back_n(renderStepSamplerCount, kUninitializedSamplerDesc);
```

Good:

```cpp
// Determine how many render step samplers were used by comparing the binding value
// against paintTextureCount, taking into account the binding requirements. We
// assume and do not anticipate the render steps to use immutable samplers.
int renderStepSamplerCount = bindingReqs.fSeparateTextureAndSamplerBinding
        ? (binding - paintTextureCount) / 2
        : binding - paintTextureCount;
static constexpr SamplerDesc kUninitializedSamplerDesc {};
outDescs->push_back_n(renderStepSamplerCount, kUninitializedSamplerDesc);
```

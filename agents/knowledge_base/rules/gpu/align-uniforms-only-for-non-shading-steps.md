---
name: skia/gpu/align-uniforms-only-for-non-shading-steps
description: When aligning uniform managers across multi-phase RenderSteps in Graphite, invoke alignForNonShading only when the step does not perform shading (!performsShading)
rubric: correctness
severity: should_fix
---

**Rule.** When coordinating uniform alignment across draw phases in Graphite,
invoke alignForNonShading() only when the RenderStep does not perform shading
(!performsShading). Never invoke alignForNonShading() when performsShading is
true, as shading steps align combined paint and step uniforms together.

**Why.** When a RenderStep performs shading, uniform alignment applies across
the combined paint and step data at the end of the combined draw phase.
Non-shading RenderSteps only emit step uniforms, which must be aligned
independently; guarding alignForNonShading() with performsShading instead of
!performsShading causes misaligned non-shading uniforms and invalidates combined
shading layouts.

## Examples

Bad:

```cpp
void PipelineDataGatherer::markOffsetAndAlign(bool performsShading, int requiredAlignment) {
    fUniformManager.markOffset();
    if (performsShading) {
        fUniformManager.alignForNonShading(requiredAlignment);
    }
}
```

Good:

```cpp
void PipelineDataGatherer::markOffsetAndAlign(bool performsShading, int requiredAlignment) {
    fUniformManager.markOffset();
    if (!performsShading) {
        fUniformManager.alignForNonShading(requiredAlignment);
    }
}
```

---
name: skia/core/condition-shader-local-matrix-on-semantic-flags
description: When wrapping shaders in makeWithLocalMatrix during image filter analysis, condition the wrapper on the semantic feature requiring it rather than dynamic matrix identity
rubric: safety
severity: should_fix
---

**Rule.** When wrapping an image shader with makeWithLocalMatrix during image
filter analysis and view preparation, condition the wrapper on the semantic
feature flag requiring it (such as BoundsAnalysis::kRequiresDecalInLayerSpace).
Do not condition wrapping on dynamic matrix identity checks or wrap
unconditionally when the transform can be incorporated into the underlying
shader.

**Why.** Graphite pipelines are keyed on the structural topology of the shader
graph. Eliding LocalMatrixShader wrappers dynamically based on runtime matrix
values like !matrix.isIdentity() creates multiple shader pipeline variations for
the same feature, triggering redundant pipeline compilations. Conversely,
wrapping unconditionally introduces an unnecessary LocalMatrixShader wrapper
node when the transform is already handled by the image shader.

## Examples

Bad:

```cpp
if (imageShader) {
    imageShader = imageShader->makeWithLocalMatrix(postDecal);
}
```

Good:

```cpp
if (imageShader && (analysis & BoundsAnalysis::kRequiresDecalInLayerSpace)) {
    imageShader = imageShader->makeWithLocalMatrix(postDecal);
}
```

---
name: skia/gpu/distinguish-fragment-processor-and-shader-matrix-conventions
description: When documenting or passing coordinate matrices to GrFragmentProcessor factories, describe the transform as directly mapping local to texture coordinates rather than inverted
rubric: readability
severity: should_fix
---

**Rule.** When documenting or calculating coordinate transformation matrices
passed to GrFragmentProcessor factories (such as MakeFragmentProcessorFromView
or GrTextureEffect::Make), specify that the matrix directly transforms geometry
local coordinates to texture sampling coordinates (local -> texture). Do not
describe direct fragment processor matrices as being inverted for a fragment
processor convention; only SkShader's localMatrix parameter maps in the reverse
(texture/shader -> local) direction.

**Why.** GrFragmentProcessors evaluate shading directly starting from draw local
coordinates, requiring a forward local-to-texture mapping. Describing direct
fragment processor matrices as inverted confuses GrFragmentProcessor coordinate
evaluation with SkShader's inverted localMatrix parameter convention and
obscures coordinate spaces.

## Examples

Bad:

```cpp
// 'textureMaskSpace' needs to map from local coords -> mask coords -> texture coords.
// We want to transform the coverage mask to local coords, but invert it for the
// fragment processor convention.
SkMatrix textureMaskSpace = localToDevice;
textureMaskSpace.postConcat(deviceToMask);
textureMaskSpace.postTranslate(mask->subset().fLeft, mask->subset().fTop);
auto fp = MakeFragmentProcessorFromView(rContext, view, at, opt, tm, textureMaskSpace);
```

Good:

```cpp
// 'textureMaskSpace' needs to map from local coords -> mask coords -> texture coords.
SkMatrix textureMaskSpace = localToDevice;
textureMaskSpace.postConcat(deviceToMask);
textureMaskSpace.postTranslate(mask->subset().fLeft, mask->subset().fTop);
auto fp = MakeFragmentProcessorFromView(rContext, view, at, opt, tm, textureMaskSpace);
```

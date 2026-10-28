---
name: skia/gpu/prefer-makefragmentprocessorfromview-over-skimage-ganesh
description: When creating a GrFragmentProcessor from an existing GrSurfaceProxyView in Ganesh, call MakeFragmentProcessorFromView directly instead of constructing a temporary SkImage_Ganesh wrapper
rubric: readability
severity: should_fix
---

**Rule.** When creating a GrFragmentProcessor to sample an existing
GrSurfaceProxyView, call skgpu::ganesh::MakeFragmentProcessorFromView directly.
Do not wrap the GrSurfaceProxyView in a temporary SkImage_Ganesh instance to
call skgpu::ganesh::AsFragmentProcessor.

**Why.** AsFragmentProcessor is designed for generic SkImage instances and
simply unwraps Ganesh-backed images to delegate to
MakeFragmentProcessorFromView. When a GrSurfaceProxyView is already available,
constructing a temporary SkImage_Ganesh wrapper adds unnecessary object
construction, reference count churn, and boilerplate indirection.

## Examples

Bad:

```cpp
const SkImage_Ganesh image(sk_ref_sp(mask->getContext()),
                           mask->uniqueID(),
                           std::move(view),
                           mask->colorInfo());
std::unique_ptr<GrFragmentProcessor> fp = skgpu::ganesh::AsFragmentProcessor(
    sdc, &image, sampling, tileModes, textureMaskSpace, &maskSubset, &maskSubset);
```

Good:

```cpp
std::unique_ptr<GrFragmentProcessor> fp = skgpu::ganesh::MakeFragmentProcessorFromView(
    this->recordingContext(), std::move(view), mask->alphaType(), sampling, tileModes,
    textureMaskSpace, &maskSubset, &maskSubset);
```

---
name: skia/image-assert-both-gpu-backends-for-non-gpu-images
description: When asserting or verifying that an image or surface is not GPU-backed, check both !isGaneshBacked() and !isGraphiteBacked() rather than checking only Ganesh
rubric: correctness
severity: should_fix
---

**Rule.** When asserting or checking that an SkImage, SkSpecialImage, or
SkSurface is not GPU-backed (such as in raster, PDF, or CPU-only routines that
require host memory), check both !isGaneshBacked() and !isGraphiteBacked(), or
verify !isTextureBacked() where supported. Do not check only !isGaneshBacked().

**Why.** Skia supports two GPU rendering backends: Ganesh and Graphite. Checking
only !isGaneshBacked() allows Graphite-backed images and surfaces to pass
CPU-only assertions or guards, leading to invalid pixel access, assertion
failures, or crashes when attempting to read raster memory.

## Examples

Bad:

```cpp
// Flawed: Graphite-backed images pass this assertion and crash when accessing CPU pixels.
SkASSERT(!srcImg->isGaneshBacked());
```

Good:

```cpp
// Correct: Verify the resource is not backed by either GPU backend.
SkASSERT(!srcImg->isGaneshBacked() && !srcImg->isGraphiteBacked());
// Or, where available on SkImage / SkImage_Base:
SkASSERT(!srcImg->isTextureBacked());
```

---
name: skia/gpu/use-none-mipmap-for-dst-read-samplers
description: When creating samplers for destination reads or in-shader blending, configure SkMipmapMode::kNone rather than matching the target texture's mipmapped status
rubric: correctness
severity: should_fix
---

**Rule.** When creating or configuring samplers for destination texture reads
(such as sampling a render target or dst-copy texture for in-shader blending),
always specify SkMipmapMode::kNone. Do not query or inherit the destination
target's mipmapped status to select a mipmap mode.

**Why.** Destination reads always sample 1:1 with screen coordinates in pixel
space, so higher mip levels are either not generated or irrelevant. Enabling
mipmapped sampling on destination textures can trigger invalid LOD calculations
or sample stale, ungenerated mip levels instead of current render target
contents.

## Examples

Bad:

```cpp
SkMipmapMode mipmapMode = target->mipmapped() == Mipmapped::kYes ? SkMipmapMode::kNearest
                                                                 : SkMipmapMode::kNone;
auto sampler = resourceProvider->findOrCreateCompatibleSampler(
        {{SkFilterMode::kNearest, mipmapMode}, SkTileMode::kClamp});
```

Good:

```cpp
// Destination reads sample 1:1 with screen pixels, so mipmapping is never used.
auto sampler = resourceProvider->findOrCreateCompatibleSampler(
        {SkFilterMode::kNearest, SkTileMode::kClamp});
```

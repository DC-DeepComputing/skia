---
name: skia/api/document-image-derivation-domain-and-rasterization
description: When introducing or modifying SkImage derivation methods, explicitly document backend domain retention, deferred rasterization, and context/recorder parameter requirements
rubric: readability
severity: should_fix
---

**Rule.** When declaring or modifying image derivation methods in public headers
(such as SkImage::make*), explicitly document whether the resulting SkImage
retains the source image's backend domain (raster, Ganesh, Graphite) and whether
deferred or lazy images are eagerly rasterized. Additionally, explicitly
document context or recorder parameter requirements (specifying when a
context/recorder is required versus when it must be nullptr) and related failure
conditions.

**Why.** Callers of image transformation and derivation methods rely on
predictable backend domain affinity (raster vs. GPU) and execution timing.
Omitting domain retention semantics, eager rasterization behavior, or
context/recorder compatibility requirements leaves callers uncertain whether
operations cause expensive CPU-to-GPU transfers, VRAM allocations, or unexpected
nullptr failures due to mismatched recorder contexts.

## Examples

Bad:

```cpp
/**
 * Create a new image by copying this image, scaling to fit the ImageInfo's dimensions,
 * and converting the pixels into the ImageInfo's ColorInfo.
 * If the image was not created with a graphite Recorder, pass null for it.
 *
 * return nullptr if the requested ColorInfo is not supported, or its dimensions are out of range.
 */
sk_sp<SkImage> makeScaled(skgpu::graphite::Recorder*,
                          const SkImageInfo&,
                          const SkSamplingOptions&) const;
```

Good:

```cpp
/**
 * Create a new image by copying this image and scaling to fit the ImageInfo's dimensions
 * and converting the pixels into the ImageInfo's ColorInfo.
 * This is done retaining the domain (backend) of the image (e.g. gpu, raster).
 * Deferred images (lazy, picture) might be rasterized.
 *
 * The Recorder parameter is required if the original image was created on a graphite Recorder,
 * but must be nullptr if it was created in some other way (e.g. GrContext, raster, deferred).
 *
 * return nullptr if the requested ColorInfo is not supported, its dimensions are out of range,
 * or if the recorder parameter does not match the image's backend.
 */
sk_sp<SkImage> makeScaled(skgpu::graphite::Recorder*,
                          const SkImageInfo&,
                          const SkSamplingOptions&) const;
```

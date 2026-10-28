---
name: skia/gpu/gpu-fallback-all-unused-texture-proxy-slots
description: When populating fixed-size texture proxy arrays for multi-planar shaders, iterate across all slots to assign fallback proxies to unused entries
rubric: correctness
severity: should_fix
---

**Rule.** When preparing fixed-size texture proxy arrays for multi-planar
shaders (such as YUVA ImageData), iterate over all slots to ensure every
unpopulated or null proxy is replaced with a valid fallback proxy (such as the Y
plane proxy). Do not assume only a specific channel or trailing index (such as
alpha at index 3) can be missing.

**Why.** Multi-planar shader pipelines with fixed sampler bindings validate that
every texture proxy slot in the descriptor array is non-null before gathering
pipeline data. Leaving any intermediate or unused plane proxy unpopulated causes
pipeline validation to fail and triggers a fallback to an error color.

## Examples

Bad:

```cpp
// Hardcoding a check only for alpha leaves unpopulated chroma planes null
if (!SkYUVAInfo::HasAlpha(yuvaInfo.planeConfig())) {
    imgData.fTextureProxies[3] = imgData.fTextureProxies[0];
}
```

Good:

```cpp
// Iterate across all slots to ensure every unused proxy receives a valid fallback
for (int tex = 0; tex < 4; ++tex) {
    if (!imgData.fTextureProxies[tex]) {
        imgData.fTextureProxies[tex] = imgData.fTextureProxies[0];
    }
}
```

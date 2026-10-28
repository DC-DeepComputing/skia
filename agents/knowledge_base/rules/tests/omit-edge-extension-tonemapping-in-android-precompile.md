---
name: skia/tests/omit-edge-extension-tonemapping-in-android-precompile
description: When generating Android RenderEngine precompilation combinations, do not pair RE_EdgeExtensionEffect with tone mapping or crosstalk shaders; wrap only simple image draws
rubric: correctness
severity: should_fix
---

**Rule.** In Android RenderEngine precompilation combinations, do not pair
RE_EdgeExtensionEffect with HDR tone mapping (such as RE_MouriMap_TonemapEffect)
or crosstalk effects. Only apply RE_EdgeExtensionEffect directly to simple image
draws, optionally wrapped in a linear effect (RE_LinearEffect).

**Why.** Real-world Android RenderEngine pipeline traces show that edge
extension is only evaluated directly on simple image draws and never combined
with HDR tonemapping or crosstalk. Generating precompilation permutations with
tone mapping models non-existent Android workloads and creates dead pipeline
variants.

## Examples

Bad:

```cpp
// Do not pair edge extension with tonemapping or crosstalk:
sk_sp<PrecompileShader> wrapped = wrap_in_MouriMapToneMap(effectManager, hdrImg);
sk_sp<PrecompileShader> edgeEffect = wrap_in_edge_extension(effectManager, std::move(wrapped));
```

Good:

```cpp
// Edge extension only wraps simple image draws (optionally wrapped in a linear effect):
sk_sp<PrecompileShader> edgeEffect = wrap_in_edge_extension(effectManager, hdrImg);
sk_sp<PrecompileShader> linear = wrap_in_linear_effect(
    effectManager, linearEffect, std::move(edgeEffect), SkColorSpace::MakeSRGBLinear());
```

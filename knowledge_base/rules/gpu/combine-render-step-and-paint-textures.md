---
name: skia/gpu/combine-render-step-and-paint-textures
description: When recording texture bindings for Graphite render steps, bind a combined TextureDataBlock containing both paint and RenderStep textures rather than binding only paint textures
rubric: correctness
severity: should_fix
---

**Rule.** When binding sampled textures for draw steps in Graphite (such as in
DrawPass or DrawList recording), bind a combined TextureDataBlock containing
both paint textures and render-step textures. Never bind paint textures alone or
drop textures supplied by RenderStep::writeUniformsAndTextures().

**Why.** Certain RenderSteps (such as text glyph rendering and coverage masks)
supply sampled textures directly through writeUniformsAndTextures(). Binding
only paint textures discards step-specific resources, causing texture-dependent
render steps to sample invalid or missing textures at execution time.

## Examples

Bad:

```cpp
// Bad: Binding only paint textures drops step textures written by RenderStep.
step->writeUniformsAndTextures(draw.drawParams(), gatherer);
TextureDataCache::Index textureIndex = performsShading ? paintTexturesIndex
                                                       : TextureDataCache::kInvalidIndex;
```

Good:

```cpp
// Good: Combine paint textures and render-step textures into a single TextureDataBlock.
step->writeUniformsAndTextures(draw.drawParams(), gatherer);
auto [stepUniforms, combinedTextures] = gatherer->endCombinedData(performsShading);
TextureDataCache::Index textureIndex = combinedTextures
        ? textureDataCache->insert(combinedTextures)
        : TextureDataCache::kInvalidIndex;
```

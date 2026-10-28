---
name: skia/gpu/validate-color-renderability-with-colortype
description: When validating texture targets for color rendering in Graphite, verify color-type compatibility alongside Caps::isRenderable rather than checking renderability alone
rubric: correctness
severity: should_fix
---

**Rule.** When validating a texture or proxy target for color rendering in
Graphite (such as in DrawContext::Make or surface creation), do not rely solely
on Caps::isRenderable(). Verify color-type compatibility (such as via
AreColorTypeAndFormatCompatible) because Caps::isRenderable() reports true for
all render pass attachments, including non-color depth and stencil formats.

**Why.** In Graphite, Caps::isRenderable() and FormatInfo::kRenderable_Flag
indicate whether a format can be bound as an attachment in a render pass, which
encompasses depth and stencil buffers alongside color buffers. Because non-color
formats have no valid SkColorType mapping, treating any renderable texture as a
valid color target can bind incompatible depth/stencil surfaces to color
rendering pipelines.

## Examples

Bad:

```cpp
if (!caps->isRenderable(target->textureInfo())) {
    return nullptr;
}
// Target is assumed to be a valid color target without checking color-type compatibility
```

Good:

```cpp
if (!caps->isRenderable(target->textureInfo())) {
    return nullptr;
}
if (!AreColorTypeAndFormatCompatible(colorInfo.colorType(), target->format())) {
    return nullptr;
}
```

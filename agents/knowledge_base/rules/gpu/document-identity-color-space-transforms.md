---
name: skia/gpu/document-identity-color-space-transforms
description: When achieving a no-op color conversion by passing the destination color space as source, document that input color is assumed to match the destination
rubric: readability
severity: should_fix
---

**Rule.** When producing an identity or no-op color space transform by passing
the destination color space as the source color space (such as when a flag skips
color conversion), add an inline explanatory comment. Clarify that the input
color is assumed to already be in the destination color space to produce an
identity transform rather than leaving readers to wonder if the color space
assignment is inverted.

**Why.** Passing a destination color space as a source argument appears
counterintuitive or backwards to reviewers and readers inspecting color
transform pipelines. Explicitly documenting that the source color is assumed to
already match the destination clarifies that setting the source to the
destination produces a deliberate identity transform (dst -> dst) to bypass
conversion.

## Examples

Bad:

```cpp
const SkColorSpace* primitiveCS =
        fSkipColorXform ? keyContext.dstColorInfo().colorSpace()
                        : sk_srgb_singleton();
AddPrimitiveColor(keyContext, keyBuilder, gatherer, primitiveCS);
```

Good:

```cpp
// When fSkipColorXform is true, the primitive color is assumed to already
// be in the destination color space, so passing dstColorSpace as the source
// produces a deliberate identity transform.
const SkColorSpace* primitiveCS =
        fSkipColorXform ? keyContext.dstColorInfo().colorSpace()
                        : sk_srgb_singleton();
AddPrimitiveColor(keyContext, keyBuilder, gatherer, primitiveCS);
```

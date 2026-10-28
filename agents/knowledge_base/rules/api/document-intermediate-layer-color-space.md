---
name: skia/api/document-intermediate-layer-color-space
description: When configuring intermediate layers or rendering pipelines with color-space overrides, document which color space backdrop filters, restore paints, and image filters evaluate in
rubric: readability
severity: should_fix
---

**Rule.** When adding color-space or format overrides to layer or pipeline
configurations (such as SaveLayerRec::fColorSpace), document which color space
intermediate operations—such as backdrop filters, restore paints, and image
filters—are evaluated in. Clarify whether intermediate filters and blending
execute in the layer's override color space or the parent canvas color space.

**Why.** When an offscreen layer operates in a different color space from its
destination canvas, it is ambiguous whether backdrop filtering, restore paints,
and image filters execute before or after color conversion. Documenting this
contract clarifies expectations for API consumers and prevents subtle
color-shift defects or unexpected conversions during layer restoration.

## Examples

Bad:

```cpp
/** Sets fBounds, fColorSpace, and fSaveLayerFlags.
    @param colorSpace If not null, when the layer is restored, a color space
                      conversion will be applied from this color space to the
                      parent's color space.
*/
SaveLayerRec(const SkRect* bounds, const SkPaint* paint,
             const SkColorSpace* colorSpace, SaveLayerFlags flags);
```

Good:

```cpp
/** Sets fBounds, fColorSpace, and fSaveLayerFlags.
    @param colorSpace If not null, when the layer is restored, a color space
                      conversion will be applied from this color space to the
                      parent's color space. The restore paint and backdrop filters will
                      be applied in this color space.
*/
SaveLayerRec(const SkRect* bounds, const SkPaint* paint,
             const SkColorSpace* colorSpace, SaveLayerFlags flags);
```

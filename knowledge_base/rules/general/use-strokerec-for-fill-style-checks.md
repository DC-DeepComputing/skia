---
name: skia/use-strokerec-for-fill-style-checks
description: When checking whether an SkPaint renders as a fill, query SkStrokeRec(paint).isFillStyle() or SkStrokeRec(paint).getStyle() instead of comparing paint.getStyle() against kFill_Style
rubric: correctness
severity: nit
---

**Rule.** When checking whether an SkPaint will draw as a fill (such as in
canvas draw dispatchers, device renderers, or fast-path guards), query
SkStrokeRec(paint).isFillStyle() or check SkStrokeRec(paint).getStyle() ==
SkStrokeRec::kFill_Style. Do not check paint.getStyle() == SkPaint::kFill_Style
directly, because SkStrokeRec normalizes legacy SkPaint::kStrokeAndFill_Style
with zero stroke width (hairline) into kFill_Style.

**Why.** SkStrokeRec::init() normalizes SkPaint::kStrokeAndFill_Style with zero
stroke width into kFill_Style because a hairline stroke on a filled geometry has
no visual effect beyond the fill itself ('hairline + fill == fill'). Querying
paint.getStyle() directly fails to recognize hairline stroke-and-fill paints as
fills, erroneously bypassing fill fast paths or dropping valid draw operations.

## Examples

Bad:

```cpp
// Erroneously excludes hairline stroke-and-fill paints that render as fill:
if (paint.getStyle() != SkPaint::kFill_Style) {
    return nullptr;
}
```

Good:

```cpp
// SkStrokeRec normalizes hairline kStrokeAndFill_Style to kFill_Style:
if (!SkStrokeRec(paint).isFillStyle()) {
    return nullptr;
}
```

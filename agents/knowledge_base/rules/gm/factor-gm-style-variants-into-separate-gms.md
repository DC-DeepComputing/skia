---
name: skia/gm/factor-gm-style-variants-into-separate-gms
description: When authoring GM tests, size canvas dimensions to fit all rendered and magnified views without clipping, and factor style variants into separate compact GMs
rubric: testing
severity: should_fix
---

**Rule.** When authoring GM rendering tests (DEF_GM, DEF_SIMPLE_GM, or
skiagm::GM), ensure declared canvas dimensions (width, height, or getISize())
comfortably fit all rendered geometry and translated or magnified views without
clipping. When testing multiple style or parameter variants (such as stroke
caps, joins, widths, or blend modes), factor each variant into a separate
compact GM test backed by a shared helper rather than combining combinatorial
sweeps into an oversized canvas. If rendering multiple variations in one GM,
translate and space them out to prevent overdraw and clipping.

**Why.** In Skia Gold tests, canvas dimensions define the bounding box
snapshotted for visual comparison; rendering beyond declared bounds clips
geometry off-screen and leaves visual behavior untested. Factoring parameter
variants into separate compact GMs prevents excessively large canvases from
inflating Gold storage and viewer overhead, avoids duplicated loops, and
isolates regressions to specific features.

## Examples

Bad:

```cpp
// 1. Monolithic parameter sweep with undersized canvas clipping translated views:
DEF_SIMPLE_GM(closedcappedhairline, canvas, 100, 500) {
    const SkPaint::Cap caps[] = {SkPaint::kButt_Cap, SkPaint::kSquare_Cap, SkPaint::kRound_Cap};
    for (auto cap : caps) {
        // ... draw normal paths ...
    }
    // Translated by 100px: zoomed-in view is clipped outside the 100px canvas width!
    canvas->translate(100, 0);
    canvas->scale(kScale, kScale);
    for (auto cap : caps) {
        // ... draw zoomed paths ...
    }
}
```

Good:

```cpp
static void draw_hairline_contours_with_caps(SkCanvas* canvas, SkPaint::Cap cap) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    paint.setStrokeCap(cap);
    draw_hairline_paths(canvas, paint);
    // Translate and magnify within canvas bounds (400x300 fits both views)
    canvas->translate(150, 0);
    canvas->scale(kScale, kScale);
    draw_zoomed_paths(canvas, paint);
}

DEF_SIMPLE_GM(closedcappedhairline_buttcap, canvas, 400, 300) {
    draw_hairline_contours_with_caps(canvas, SkPaint::kButt_Cap);
}
DEF_SIMPLE_GM(closedcappedhairline_roundcap, canvas, 400, 300) {
    draw_hairline_contours_with_caps(canvas, SkPaint::kRound_Cap);
}
```

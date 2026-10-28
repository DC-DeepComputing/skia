---
name: skia/gm/highlight-subtle-features-in-gm-tests
description: When authoring GM tests for subtle behaviors, draw 1:1 and magnified snapshots side-by-side, highlight regions of interest, and document overlay purposes
rubric: readability
severity: should_fix
---

**Rule.** When authoring GM tests that exercise subtle or subpixel rendering
behaviors (such as hairline stroke caps, joins, or antialiasing differences),
rasterize geometry offscreen at 1:1 and draw both the native unscaled image and
a magnified snapshot side-by-side. Additionally, draw high-contrast visual
indicators (such as thin colored bounding boxes or crosshairs) around regions of
interest, keeping visual callouts distinct from test geometry and documenting
their purpose so triagers understand what expected features or boundaries they
delineate.

**Why.** Hairlines and subpixel geometries do not scale with the CTM and render
at 1 device pixel; drawing only a scaled canvas obscures either native 1x
rendering or fine details. Rendering 1:1 and magnified snapshots side-by-side
with high-contrast bounding boxes makes subtle variations obvious during Gold
triage and Viewer inspection, while documenting overlay purposes prevents
confusing test markers with intentional drawing features.

## Examples

Bad:

```cpp
// Renders paths directly or only draws a magnified snapshot without visual callouts
auto pathImg = pathSurface->makeImageSnapshot();
canvas->scale(kScale, kScale);
canvas->drawImage(pathImg, 0, 0);

// Undocumented highlight boxes leave triagers unclear on what features are expected
static void draw_highlights(SkCanvas* canvas, SkSpan<const SkPath> paths) {
    SkPaint highlightPaint;
    highlightPaint.setStyle(SkPaint::kStroke_Style);
    highlightPaint.setColor(SK_ColorRED);
    for (const auto& path : paths) {
        canvas->drawRect(highlight_box(path.getPoint(0)), highlightPaint);
    }
}
```

Good:

```cpp
// Draws 1:1 unscaled and magnified snapshots side-by-side
auto pathImg = pathSurface->makeImageSnapshot();
canvas->drawImage(pathImg, 0, 0);
canvas->scale(kScale, kScale);
canvas->drawImage(pathImg, 15, 0);

// Highlight boxes around contour endpoints where capped edges are expected
static void draw_highlights(SkCanvas* canvas, SkSpan<const SkPath> paths) {
    SkPaint highlightPaint;
    highlightPaint.setStyle(SkPaint::kStroke_Style);
    highlightPaint.setColor(SK_ColorRED);
    for (const auto& path : paths) {
        canvas->drawRect(highlight_box(path.getPoint(0)), highlightPaint);
    }
}
```

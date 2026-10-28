---
name: skia/tests/avoid-blur-mask-filters-in-task-graph-tests
description: When testing Graphite task graphs or layer dependencies, use simple solid-color draws instead of blur mask filters handled inline by AnalyticBlurRenderStep
rubric: readability
severity: should_fix
---

**Rule.** In Graphite task-graph, resource usage, or layer dependency unit
tests, use simple solid-color draws rather than blur mask filters or complex
paint effects to verify graph structure. Blurred basic geometries (rectangles,
rounded rectangles, and circles) execute inline via AnalyticBlurRenderStep
without creating offscreen render passes or altering the task graph.

**Why.** Analytic blur render steps in Graphite evaluate blur effects directly
within the primary render pass rather than generating secondary task nodes.
Using blur mask filters adds unnecessary paint complexity, requires fuzzy
color-matching assertions in tests, and obscures what operations actually impact
the task graph.

## Examples

Bad:

```cpp
canvas->saveLayer(nullptr, &blendPaint);
SkPaint shadowPaint;
shadowPaint.setColor(SK_ColorBLUE);
shadowPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5.0f));
canvas->drawRect(SkRect::MakeXYWH(30, 15, 45, 45), shadowPaint);
canvas->restore();
```

Good:

```cpp
canvas->saveLayer(nullptr, &blendPaint);
SkPaint solidPaint;
solidPaint.setColor(0xCC33FF99);
canvas->drawRect(SkRect::MakeXYWH(30, 15, 45, 45), solidPaint);
canvas->restore();
```

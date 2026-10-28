---
name: skia/gm/prefer-def-simple-gm-when-rendering-cannot-fail
description: When defining GM rendering tests that have no fallible setup or skip conditions, use DEF_SIMPLE_GM instead of DEF_SIMPLE_GM_CAN_FAIL
rubric: readability
severity: should_fix
---

**Rule.** When authoring GM rendering tests that do not have fallible setup,
skip conditions, or error reporting, register them using DEF_SIMPLE_GM (or
DEF_SIMPLE_GM_BG) instead of DEF_SIMPLE_GM_CAN_FAIL. Remove unused fallible
calls (such as allocating unused scratch surfaces) rather than keeping
DEF_SIMPLE_GM_CAN_FAIL and returning skiagm::DrawResult::kOk.

**Why.** DEF_SIMPLE_GM is the standard helper macro for GM rendering tests that
do not fail or skip execution; it avoids the boilerplate errorMsg parameter and
explicit DrawResult::kOk return. DEF_SIMPLE_GM_CAN_FAIL implies the test can
legitimately fail or skip, obscuring test intent when rendering cannot fail.

## Examples

Bad:

```cpp
DEF_SIMPLE_GM_CAN_FAIL(hairlines_buttcap, canvas, errorMsg, 100, 500) {
    auto surface = canvas->makeSurface(canvas->imageInfo().makeWH(80, 80));
    if (!surface) {
        *errorMsg = "Could not make subsurface";
        return skiagm::DrawResult::kSkip;
    }
    draw_hairline_contours_with_caps(canvas, SkPaint::kButt_Cap);
    return skiagm::DrawResult::kOk;
}
```

Good:

```cpp
DEF_SIMPLE_GM(hairlines_buttcap, canvas, 100, 500) {
    draw_hairline_contours_with_caps(canvas, SkPaint::kButt_Cap);
}
```

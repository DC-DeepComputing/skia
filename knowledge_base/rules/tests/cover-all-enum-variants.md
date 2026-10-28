---
name: skia/tests/cover-all-enum-variants
description: When writing unit tests or rendering tests parameterized across an enum, test all enum values including defaults rather than omitting variants
rubric: testing
severity: should_fix
---

**Rule.** When authoring unit tests or rendering tests parameterized across an
enumeration (such as SkPaint::Cap, SkPaint::Join, or SkBlendMode), cover every
valid enum value across test cases or loops. Do not omit default, identity, or
basic variants (such as SkPaint::kButt_Cap or SkBlendMode::kSrcOver) unless
there is an explicit, documented reason why a value does not apply.

**Why.** Omitting enum variants leaves critical code paths and default
configurations untested. Exhaustively exercising all variants catches corner
cases, ensures fallback and default pipelines function properly, and prevents
regressions when shared routines dispatch across enumeration states.

## Examples

Bad:

```cpp
// Testing cap rendering but accidentally omitting default kButt_Cap
DEF_TEST(DrawHairline_SquareCap, r) { draw_hairline(r, SkPaint::kSquare_Cap); }
DEF_TEST(DrawHairline_RoundCap, r) { draw_hairline(r, SkPaint::kRound_Cap); }
```

Good:

```cpp
// Explicitly testing all enum variants including default kButt_Cap
DEF_TEST(DrawHairline_ButtCap, r) { draw_hairline(r, SkPaint::kButt_Cap); }
DEF_TEST(DrawHairline_SquareCap, r) { draw_hairline(r, SkPaint::kSquare_Cap); }
DEF_TEST(DrawHairline_RoundCap, r) { draw_hairline(r, SkPaint::kRound_Cap); }
```

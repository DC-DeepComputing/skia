---
name: skia/regression-test-arithmetic-overflow-fixes
description: When fixing bugs, numerical overflows, or fuzzer edge cases, add direct reproduction tests (DEF_TEST or GM) in Skia rather than deferring coverage
rubric: testing
severity: should_fix
---

**Rule.** When fixing bugs, numerical overflows, fuzzer reports, or transform
edge cases, include a reproduction test directly in the Skia repository (under
`tests/` or `gm/`) rather than deferring test coverage to downstream consumers
like Chromium. For math, geometry, transform, or fuzzer fixes, add a regression
unit test (`DEF_TEST` or `DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS`) exercising
exact reproducer coordinates, degenerate matrices, or extreme boundary values
(such as `numeric_limits<float>::max()`). If the fix touches rendering or
pipeline stages without straightforward assertions, add a Golden Master test
(`DEF_SIMPLE_GM` or `DEF_GM`) named after the issue ID.

**Why.** Reproduction tests prevent regressions and catch unexpected compiler,
platform, or backend discrepancies across Skia's supported configurations.
Because Skia is an independent library embedded across multiple consumers (such
as Chromium and Android), adding tests directly in Skia ensures regressions are
caught on Skia's continuous integration bots before dependency rolls, while
issue-named and fuzzer reproducers preserve triage context.

## Examples

Bad:

```cpp
// 1. Deferring test coverage to downstream consumer in commit message:
// Commit message:
A follow-up CL with test coverage can be found in https://crrev.com/c/6961943

// 2. Math/geometry overflow fix without boundary unit test:
pts[i] = (p0 + 2.f * p1 + p2) * 0.25f;

// 3. Rendering pipeline fix for extreme dimensions without GM:
fx = std::max(std::min(fx, kInt32Max), kInt32Min);

// 4. Handling transform or fuzzer edge cases without reproducer test:
if (matrix.rectStaysRect() && !matrix.preservesRightAngles()) {
    return false; // Fast bailout added without test
}
```

Good:

```cpp
// 1. Regression unit test added directly in Skia (tests/):
DEF_TEST(RustPngCodec_crbug445556737, r) {
    sk_sp<SkImage> image = DecodeLastFrame(r, "images/crbug445556737.png");
    REPORTER_ASSERT(r, image && image->width() == 5 && image->height() == 5);
}

// 2. Boundary value unit test for math/geometry fixes:
DEF_TEST(GrPathUtils_bug451448218, r) {
    auto pts = GrPathUtils::generateQuadraticPoints(
        {std::numeric_limits<float>::min(), std::numeric_limits<float>::max()},
        {std::numeric_limits<float>::min() / 2, std::numeric_limits<float>::max() / 4},
        {std::numeric_limits<float>::min() / 4, std::numeric_limits<float>::max() / 2});
    REPORTER_ASSERT(r, std::all_of(pts.begin(), pts.end(), [](const SkPoint& p) { return p.isFinite(); }));
}

// 3. Visual reproduction GM named after issue ID:
DEF_SIMPLE_GM(crbug_404394639, canvas, 500, 500) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(500, 40000));
    canvas->drawImage(surface->makeImageSnapshot(), 0, 0);
}

// 4. Degenerate transform or fuzzer reproducer test:
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(BlurDegenerateAffineFuzzer, r, ctxInfo, CtsEnforcement::kNever) {
    SkMatrix m = SkMatrix::MakeAll(0.f, 0.000001f, 0.f, 0.000001f, 0.f, 0.f, 0.f, 0.f, 1.f);
    REPORTER_ASSERT(r, !m.preservesRightAngles() && m.rectStaysRect());
    canvas->concat(m);
    canvas->drawRect(SkRect::MakeWH(100.f, 100.f), paint);
}
```

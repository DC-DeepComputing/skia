---
name: skia/unit-test-curve-subdivision-and-flattening
description: When introducing or modifying curve subdivision, flattening, or geometry-mapping algorithms, add unit tests exercising core calculations across standard and degenerate curves
rubric: testing
severity: should_fix
---

**Rule.** When introducing or modifying mathematical routines for curve
subdivision, geometry flattening, or reparameterization, add unit tests
exercising the core estimation and chopping routines. Test both standard curves
and edge-case or degenerate geometries (such as coincident points, collinear
control points, zero-length segments, or extreme curvatures).

**Why.** Curve subdivision and flattening routines rely on sensitive
floating-point estimations and parameter mappings that are prone to numerical
instability, division by zero, or underflow. Direct unit tests covering
degenerate and edge-case geometries verify numerical robustness and prevent
rendering regressions that high-level visual tests miss.

## Examples

Bad:

```cpp
// Introducing curve subdivision or flattening without unit tests exercising the math:
class Flattener {
public:
    void flatten(SkSpan<const SkPoint> pts, float tolerance, Polyline* out);
    // No unit test verifying subdivision accuracy or handling of degenerate collinear curves.
};
```

Good:

```cpp
// tests/FlattenTest.cpp:
DEF_TEST(CurveFlattening, reporter) {
    // Test standard quadratic curve
    CheckFlattenedPath(reporter, SkPathBuilder().moveTo(10, 10).quadTo(50, 100, 90, 10).detach(), "StandardQuad");

    // Test degenerate curves (collinear, zero length, coincident control points)
    CheckFlattenedPath(reporter, SkPathBuilder().moveTo(0, 0).quadTo(50, 50, 100, 100).detach(), "Collinear");
    CheckFlattenedPath(reporter, SkPathBuilder().moveTo(20, 20).quadTo(20, 20, 20, 20).detach(), "ZeroLength");
}
```

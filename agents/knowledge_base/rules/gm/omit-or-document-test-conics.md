---
name: skia/gm/omit-or-document-test-conics
description: When authoring paths in tests or GMs, omit arbitrary conicTo segments if standard curves suffice, or document the intended shape if conics are necessary
rubric: readability
severity: should_fix
---

**Rule.** When constructing path geometry for tests and GMs, avoid adding
arbitrary conicTo segments alongside lines, quads, and cubics unless the test
specifically evaluates rational quadratic curve rendering or math. If conic
curves are required, add descriptive comments explaining the intended visual
shape and weight parameter, as conic geometries are difficult to visualize and
verify from numeric coordinates alone.

**Why.** Conic curves require both control points and a weight factor, making
their trajectory and curvature difficult to visualize mentally or verify during
code review without explicit documentation. Including arbitrary conics in
general drawing tests increases complexity and maintenance overhead without
providing unique geometric coverage beyond standard quad and cubic Bézier
curves.

## Examples

Bad:

```cpp
// Arbitrary conic curve added without documentation alongside standard curves
pb.moveTo(5.5, 5.5).lineTo(5.5, 10.5).lineTo(10.5, 10.5).lineTo(10.5, 5.5).close();
pb.moveTo(20.5, 5.5).quadTo(25, 15, 30.5, 5.5).close();
pb.moveTo(40.5, 5.5).cubicTo(35, 25, 45, 20, 50.5, 5.5).close();
pb.moveTo(60.5, 5.5).conicTo(65, 25, 70.5, 5.5, 50.f).close();
```

Good:

```cpp
// Option 1: Omit conicTo when lines, quads, and cubics already provide sufficient coverage
pb.moveTo(5.5, 5.5).lineTo(5.5, 10.5).lineTo(10.5, 10.5).lineTo(10.5, 5.5).close();
pb.moveTo(20.5, 5.5).quadTo(25, 15, 30.5, 5.5).close();
pb.moveTo(40.5, 5.5).cubicTo(35, 25, 45, 20, 50.5, 5.5).close();

// Option 2: When conics are necessary, document the target shape and weight
// A circular arc segment spanning 60 degrees using weight = cos(30 deg)
pb.moveTo(60.5, 5.5).conicTo(65, 25, 70.5, 5.5, 0.866f).close();
```

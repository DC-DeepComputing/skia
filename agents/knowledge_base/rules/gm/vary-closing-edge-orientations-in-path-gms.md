---
name: skia/gm/vary-closing-edge-orientations-in-path-gms
description: When authoring GM rendering tests for path contours or hairlines, vary closing segment orientations across directions instead of orienting all closing edges identically
rubric: testing
severity: should_fix
---

**Rule.** When authoring visual GM rendering tests for path contours, hairlines,
or contour-closing behavior, test closing segments across different orientations
(such as horizontal, vertical, and diagonal edges along different sides) rather
than orienting all closing edges along a single direction. Ensure test suites
include contours that close along vertical sides or bottoms as well as top
edges.

**Why.** Rasterization, edge-sorting, and antialiasing algorithms frequently
treat horizontal, vertical, and diagonal edges differently due to scanline
ordering and orientation-specific cap or join geometry calculations. If all test
contours close along the same direction (such as only along the top),
directional edge bugs, cracks, or cap misalignments along sides and bottoms will
go undetected.

## Examples

Bad:

```cpp
// Bad: All contours close along the top edge, leaving vertical/bottom edge rasterization untested
pb.moveTo(5.5, 5.5).lineTo(5.5, 10.5).lineTo(10.5, 10.5).lineTo(10.5, 5.5).close();
pb.moveTo(20.5, 5.5).quadTo(25, 15, 30.5, 5.5).close();
pb.moveTo(40.5, 5.5).cubicTo(35, 25, 45, 20, 50.5, 5.5).close();
```

Good:

```cpp
// Good: Contours close along different orientations (e.g. side/vertical edges as well as top)
auto lineOnOpen = SkPathBuilder().lineTo(0, 5).lineTo(5, 5).lineTo(5, 0).detach(); // closes top
auto quadOnOpen = SkPathBuilder().quadTo(15, 5, 0, 10).detach();                     // closes left side
auto cubicOnOpen = SkPathBuilder().cubicTo(-5, 0, -5, 5, 0, 10).detach();            // closes left side
auto lineOnClosed = SkPathBuilder(lineOnOpen).close().detach();
auto quadOnClosed = SkPathBuilder(quadOnOpen).close().detach();
auto cubicOnClosed = SkPathBuilder(cubicOnOpen).close().detach();
```

---
name: skia/omit-redundant-consecutive-moveto
description: When constructing paths with SkPathBuilder, omit redundant consecutive moveTo calls without intervening drawing verbs, as SkPathBuilder overwrites the preceding move point
rubric: readability
severity: should_fix
---

**Rule.** When building paths using SkPathBuilder, do not issue consecutive
moveTo or rMoveTo calls without intervening drawing verbs (such as lineTo,
quadTo, conicTo, cubicTo, or close). Remove the preceding moveTo call when it is
immediately replaced by a subsequent move.

**Why.** SkPathBuilder overwrites the preceding move point in place when moveTo
is invoked while the last verb is already kMove rather than starting a new
contour. An isolated moveTo immediately followed by another moveTo is dead code
that has no effect on the resulting path geometry.

## Examples

Bad:

```cpp
// Bad: First moveTo is dead code overwritten by the immediately following moveTo
SkPath path = SkPathBuilder()
                      .moveTo(5.5f, 40.0f)
                      .moveTo(5.5f, 45.5f)
                      .lineTo(5.5f, 50.5f)
                      .lineTo(10.5f, 60.5f)
                      .detach();
```

Good:

```cpp
// Good: Omit the redundant initial moveTo call
SkPath path = SkPathBuilder()
                      .moveTo(5.5f, 45.5f)
                      .lineTo(5.5f, 50.5f)
                      .lineTo(10.5f, 60.5f)
                      .detach();
```

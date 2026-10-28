---
name: skia/modules/avoid-tobe-on-arrays-in-canvaskit-tests
description: In CanvasKit JavaScript unit tests, do not use .toBe() on arrays or typed arrays; verify length and compare elements using expectArrayCloseTo()
rubric: correctness
severity: should_fix
---

**Rule.** In CanvasKit JavaScript unit tests, do not use .toBe() to assert
equality on arrays or typed arrays (such as Float32Array, color vectors, or
transformation matrices). Verify array lengths and compare elements with
numerical tolerance using assertion helpers like expectArrayCloseTo() or
expectColorCloseTo().

**Why.** In Jasmine unit tests, .toBe() evaluates reference equality (===),
which always fails for distinct array or TypedArray instances even when their
contents are identical. Furthermore, floating-point coordinates, matrices, and
color components are susceptible to minor numerical precision variances and
require element-by-element comparison with an appropriate numerical tolerance.

## Examples

Bad:

```cpp
expect(animation.getColorSlot('FillsGroup')).toBe(CanvasKit.RED);
expect(damageRect).toBe(Float32Array.of(0, 0, 800, 600, 999));
```

Good:

```cpp
expectArrayCloseTo(animation.getColorSlot('FillsGroup'), CanvasKit.RED, 4);
expectArrayCloseTo(damageRect, Float32Array.of(0, 0, 800, 600, 999), 4);
```

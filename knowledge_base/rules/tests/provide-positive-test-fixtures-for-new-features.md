---
name: skia/tests/provide-positive-test-fixtures-for-new-features
description: When testing new features or APIs, provide dedicated test fixtures exercising positive paths and rendering rather than deferring tests or asserting only negative cases
rubric: testing
severity: should_fix
---

**Rule.** When introducing new features, APIs, or bindings, provide dedicated
test fixtures or test cases that genuinely exercise positive paths and rendering
in the same CL. Do not defer positive test coverage to future CLs with
placeholder comments, nor rely solely on negative checks against unconfigured or
missing assets.

**Why.** Testing only negative outcomes or unconfigured fallbacks leaves core
feature logic, type conversions, and rendering pipelines unexercised in CI,
masking implementation defects. Providing dedicated, properly configured test
assets ensures positive paths and rendering behave correctly from the initial
landing.

## Examples

Bad:

```cpp
// json is not configured with slots, we should add a seperate gm that tests if
// properties are set correctly
animation.setColorSlot('FillsGroup', CanvasKit.RED);
animation.setScalarSlot('Opacity', 0);
expect(animation.getColorSlot('Bad ID')).toBeFalsy();
```

Good:

```cpp
// Provide a dedicated configured asset exercising positive paths and rendering
const animation = CanvasKit.MakeManagedAnimation(promises[0]);
animation.setColorSlot('FillsGroup', CanvasKit.RED);
animation.setScalarSlot('Opacity', 0);
expect(animation.getColorSlot('StrokeGroup')).toBeTruthy();
expect(animation.getScalarSlot('Opacity')).toBeTruthy();
animation.seek(0.5);
animation.render(canvas, bounds);
```

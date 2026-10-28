---
name: skia/modules/use-it-over-gm-without-canvas-draws
description: In CanvasKit tests, ensure gm() tests draw distinct non-overlapping shapes, and use it() without canvas or done parameters for non-rendering logic
rubric: testing
severity: should_fix
---

**Rule.** In CanvasKit tests (`modules/canvaskit/tests/`), ensure Golden Master
tests (`gm(...)`) actually draw content onto the provided canvas rather than
only asserting state with unit assertions, and translate or space out multiple
shapes or progression paths so they do not overlap and obscure each other in
Skia Gold baselines. Conversely, use Jasmine unit tests (`it(...)`) for
non-rendering logic or API data verification when visual inspection is not
required, and do not declare unused parameters (such as `canvas` or redundant
`done` completion callbacks in synchronous tests) in `it(...)` callbacks.

**Why.** In CanvasKit tests, `gm(...)` allocates a canvas surface and uploads
snapshots to Skia Gold; omitting canvas draw calls leaves the canvas blank,
while drawing multiple shapes at identical coordinates causes overlapping
overdraw that obscures individual shapes during visual baseline inspection.
Conversely, `it(...)` executes pure JavaScript assertions without providing a
canvas; declaring an unused `canvas` or unneeded `done` parameter causes Jasmine
to treat the test as asynchronous, risking misleading test timeouts or dead
code.

## Examples

Bad:

```cpp
// Bad: drawing multiple shapes at identical coordinates obscures geometry in Gold baselines
gm('Can_Interpolate_Path', (canvas) => {
  path2.addRect([20, 20, 40, 40]);
  canvas.drawPath(path1, paint);
  canvas.drawPath(path2, paint);
  canvas.drawPath(path3, paint); // Overlaps earlier paths at the same coordinates
});

// Bad: using gm() without drawing to canvas leaves the Gold snapshot blank
gm('Interpolate_Paths', (canvas) => {
  expect(p1.isInterpolatable(p2)).toBe(true);
  const p3 = p1.interpolate(p2, 0);
  expect(p3).toBeTruthy();
  // Does not draw anything to canvas!
});

// Bad: declaring unused canvas or redundant done callbacks in synchronous it() tests
it('paragraph_rounding_hack', (canvas) => {
  const paraStyle = new CanvasKit.ParagraphStyle({...});
  expect(paraStyle.applyRoundingHack).toEqual(true);
});

it('can compute ImageFilter filterBounds', (done) => {
  const blurIF = CanvasKit.ImageFilter.MakeBlur(5, 10, CanvasKit.TileMode.Clamp, null);
  expect(blurIF.filterBounds(rect)[0]).toEqual(35);
  done();
});
```

Good:

```cpp
// Good: translate or space out shapes across the canvas so each is visually distinct
gm('Can_Interpolate_Path', (canvas) => {
  path2.addRect([20, 20, 40, 40]);
  canvas.drawPath(path1, paint);
  canvas.save();
  canvas.translate(40, 0);
  canvas.drawPath(path2, paint);
  canvas.translate(40, 0);
  canvas.drawPath(path3, paint);
  canvas.restore();
});

// Good: use it() without parameters for synchronous API, data, and logic validation
it('should provide correct level data from getBidiRegions', () => {
  const regions = CanvasKit.Bidi.getBidiRegions(sampleText, CanvasKit.TextDirection.LTR);
  expect(regions.length).toEqual(10);
});

it('paragraph_rounding_hack', () => {
  const paraStyle = new CanvasKit.ParagraphStyle({...});
  expect(paraStyle.applyRoundingHack).toEqual(true);
});

// Good: use gm() to supply a canvas and capture rendered visual output to Skia Gold
gm('skottie_textedit', (canvas, promises) => {
  const animation = CanvasKit.MakeManagedAnimation(promises[0]);
  animation.render(canvas, bounds);
  expect(animation.dispatchEditorKey('Backspace')).toBeTrue();
  animation.delete();
}, editPromise);
```

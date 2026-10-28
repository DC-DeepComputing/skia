---
name: skia/modules/canvaskit-no-spaces-in-gm-names
description: When naming CanvasKit Golden Master tests via gm() or multipleCanvasGM(), do not include spaces; use underscores and alphanumeric characters instead
rubric: readability
severity: should_fix
---

**Rule.** When defining CanvasKit Golden Master (GM) tests using gm(),
multipleCanvasGM(), or related helpers (fgm, force_gm, fmultipleCanvasGM), do
not include spaces or whitespace in the test name string. Use underscores and
alphanumeric characters to delimit words instead.

**Why.** In CanvasKit's test harness, GM test names serve as unique identifiers
for Skia Gold baselines, output image filenames, and test runner filters.
Passing a name containing spaces causes util.js to throw a runtime error
('<name> cannot contain spaces') during test execution.

## Examples

Bad:

```cpp
// Bad: GM test name contains spaces
gm('paragraph fontSize and heightMultiplier 0', (canvas) => {
    // ...
});
```

Good:

```cpp
// Good: GM test name uses underscores instead of spaces
gm('paragraph_fontSize_and_heightMultiplier_0', (canvas) => {
    // ...
});
```

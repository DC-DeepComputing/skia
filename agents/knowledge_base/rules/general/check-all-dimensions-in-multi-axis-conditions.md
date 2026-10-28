---
name: skia/check-all-dimensions-in-multi-axis-conditions
description: When evaluating multi-axis coordinates, extents, or vector properties in compound conditions, verify that every intended dimension is checked without duplicate component checks
rubric: correctness
severity: should_fix
---

**Rule.** When validating or testing multi-dimensional properties (such as X/Y/Z
coordinates, vector components, layout dimensions, or extents) in compound
boolean conditions, ensure each intended dimension is evaluated. Do not
duplicate checks on the same coordinate axis or vector component while omitting
another dimension due to copy-paste.

**Why.** Copy-pasting component checks across multi-axis expressions frequently
causes one dimension to be tested multiple times while another is omitted
entirely. This creates silent validation gaps, incorrect layout or workgroup
sizing, and subtle logic bugs that compile without warnings.

## Examples

Bad:

```cpp
// Bug: fLocalSizeY is checked twice, omitting fLocalSizeZ
if (modifiers.fLayout.fLocalSizeX >= 0 ||
    modifiers.fLayout.fLocalSizeY >= 0 ||
    modifiers.fLayout.fLocalSizeY >= 0) {
    ...
}
```

Good:

```cpp
// Verifies each distinct dimension (X, Y, and Z)
if (modifiers.fLayout.fLocalSizeX >= 0 ||
    modifiers.fLayout.fLocalSizeY >= 0 ||
    modifiers.fLayout.fLocalSizeZ >= 0) {
    ...
}
```

---
name: skia/gpu/document-non-intuitive-coordinate-transform-directions
description: When tracking or composing coordinate transformations in a reverse direction to avoid matrix inversion overhead, document the algebraic derivation and optimization rationale
rubric: readability
severity: should_fix
---

**Rule.** When tracking or composing coordinate transformations in a
counter-intuitive or reverse direction to avoid matrix inversion overhead (such
as tracking local-to-other to map rects via inverseMapRect rather than
other-to-local), document the algebraic derivation and optimization rationale in
comments. Explicitly write out the coordinate space equation (e.g.,
(otherToDevice)^-1 * localToDevice = localToDevice) so readers can verify why
the assigned matrix represents the intended relative transform without
inversion.

**Why.** Optimizing matrix operations by tracking transforms in a reverse
direction to avoid inverting identity or simple matrices can confuse reviewers
and lead to inverted matrix assignments or incorrect comments. Explicit
algebraic derivations clarify the coordinate space transitions and prevent
maintainers from 'fixing' the seemingly backwards assignment.

## Examples

Bad:

```cpp
// Relative transform is the inverse of localToDevice
localToOther = &localToDevice;
Rect mapped = localToOther->inverseMapRect(otherBounds);
```

Good:

```cpp
// Relative transform is (otherToDevice)^-1 * localToDevice = localToDevice.
// Tracking localToOther avoids inverting otherToDevice and lets us use inverseMapRect.
localToOther = &localToDevice;
Rect mapped = localToOther->inverseMapRect(otherBounds);
```

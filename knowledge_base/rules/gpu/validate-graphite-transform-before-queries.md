---
name: skia/gpu/validate-graphite-transform-before-queries
description: When querying scale factors or coordinate bounds on Graphite Transform instances, verify transform.valid() before invoking methods like maxScaleFactor() that assert validity
rubric: correctness
severity: should_fix
---

**Rule.** Before querying scale factors or mapping coordinates on a Graphite
Transform (such as calling maxScaleFactor(), scaleFactors(), or
localAARadius()), verify that the transform is valid using transform.valid().
For non-invertible or non-finite transforms, bail out or drop the operation
early instead of invoking methods that assert validity with SkASSERT.

**Why.** Graphite's Transform class marks non-invertible or non-finite matrices
as invalid and enforces SkASSERT(this->valid()) inside scale factor and
projection accessors. Calling these methods on unverified transforms crashes
debug builds instead of allowing the invalid draw to be safely skipped.

## Examples

Bad:

```cpp
if (pathEffect) {
    float maxScale = localToDevice.maxScaleFactor();
    SkStrokeRec stroke(paint);
    ...
}
```

Good:

```cpp
if (pathEffect && localToDevice.valid()) {
    float maxScale = localToDevice.maxScaleFactor();
    SkStrokeRec stroke(paint);
    ...
}
```

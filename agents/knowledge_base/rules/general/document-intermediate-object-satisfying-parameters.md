---
name: skia/document-intermediate-object-satisfying-parameters
description: When downstream calls consume an intermediate object's property instead of an in-scope configuration parameter, document why the intermediate object already satisfies that parameter
rubric: readability
severity: nit
---

**Rule.** When invoking downstream functions that consume properties or state
from an intermediate object rather than referencing an in-scope configuration
parameter directly, add an explanatory comment. Explicitly clarify that the
intermediate object was initialized with or already incorporates the parameter,
explaining why direct forwarding or separate conversion is unnecessary.

**Why.** In multi-step setup pipelines, passing an intermediate object's derived
properties while omitting an in-scope configuration argument appears to
reviewers as an accidental omission or bug. An explanatory comment clarifies
dataflow equivalence and prevents reviewers from questioning omitted arguments
or introducing redundant conversions.

## Examples

Bad:

```cpp
// Parameter rec.fColorSpace is in scope, but newDevice's colorInfo is passed without explanation
this->internalDrawDeviceWithFilter(priorDevice,
                                   newDevice.get(),
                                   backdropAsSpan,
                                   backdropPaint,
                                   compat,
                                   newDevice->imageInfo().colorInfo(),
                                   rec.fExperimentalBackdropScale);
```

Good:

```cpp
// Using the color info of 'newDevice' is equivalent to using 'rec.fColorSpace'.
this->internalDrawDeviceWithFilter(priorDevice,
                                   newDevice.get(),
                                   backdropAsSpan,
                                   backdropPaint,
                                   compat,
                                   newDevice->imageInfo().colorInfo(),
                                   rec.fExperimentalBackdropScale);
```

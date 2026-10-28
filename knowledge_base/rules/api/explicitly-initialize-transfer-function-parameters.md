---
name: skia/api/explicitly-initialize-transfer-function-parameters
description: When defining skcms_TransferFunction constants, explicitly initialize all seven parameters ({g, a, b, c, d, e, f}) or alias existing SkNamedTransferFn constants for identical curves
rubric: readability
severity: should_fix
---

**Rule.** When declaring or defining skcms_TransferFunction constants,
explicitly initialize all seven parametric fields ({g, a, b, c, d, e, f}) rather
than relying on partial aggregate initialization. If an equivalent transfer
function curve is already defined (such as in SkNamedTransferFn), alias the
existing named constant (e.g., kA98RGB = k2Dot2) instead of defining a duplicate
literal.

**Why.** skcms_TransferFunction models a 7-parameter piecewise parametric curve.
Omitting trailing fields relies on partial aggregate initialization, which can
trigger missing-field-initializers compiler warnings and obscures mathematical
coefficients. Aliasing canonical transfer functions prevents redundant
definitions and ensures identical curves share canonical constants.

## Examples

Bad:

```cpp
namespace SkNamedTransferFn {
static constexpr skcms_TransferFunction kProPhotoRGB = {1.8f, 1.0f};
static constexpr skcms_TransferFunction kA98RGB = {2.2f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
}
```

Good:

```cpp
namespace SkNamedTransferFn {
static constexpr skcms_TransferFunction kProPhotoRGB = {1.8f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static constexpr skcms_TransferFunction kA98RGB = k2Dot2;
}
```

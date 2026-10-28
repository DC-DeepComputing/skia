---
name: skia/use-predefined-math-constants
description: When using mathematical constants like pi or sqrt(2), use predefined constants from SkFloatingPoint.h (such as SK_FloatPI) instead of local definitions or hardcoded literals
rubric: readability
severity: nit
---

**Rule.** Use predefined mathematical constants from
include/private/SkFloatingPoint.h (such as SK_FloatPI, SK_DoublePI, or
SK_FloatSqrt2) instead of hardcoding numeric approximations, using non-standard
macros (like M_PI), or defining local copies of pi.

**Why.** Skia provides centralized float and double mathematical constants in
SkFloatingPoint.h to avoid redundant definitions, eliminate precision
discrepancies across platforms, and maintain consistency where standard library
constants (such as std::numbers::pi) are not available across all target
toolchains.

## Examples

Bad:

```cpp
// Bad: defining local copies of pi or hardcoding numeric literals
static constexpr float kPi = 3.1415926535897932384626f;
const float theta = uint16_to_float(data, 1u, 35999u, 18000, 36000.f / kPi);
const float circumference = 2.0f * 3.14159265f * radius;
```

Good:

```cpp
#include "include/private/SkFloatingPoint.h"

// Good: use centralized mathematical constants from SkFloatingPoint.h
const float theta = uint16_to_float(data, 1u, 35999u, 18000u, 36000.f / SK_FloatPI);
const float circumference = 2.0f * SK_FloatPI * radius;
```

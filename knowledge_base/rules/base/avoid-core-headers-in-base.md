---
name: skia/base/avoid-core-headers-in-base
description: When authoring base code or foundational math routines, do not include core headers or place utilities in core; keep helpers in base using forward declarations
rubric: design
severity: should_fix
---

**Rule.** Do not include core headers (from include/core/ or src/core/, such as
SkTypes.h, SkString.h, or SkScalar.h) in src/base/ or include/private/base/.
Base must not depend on higher layers; define primitive math and float utilities
(such as SkIsFinite and SkIsNaN) in base headers (like
include/private/base/SkFloatingPoint.h) rather than core headers, and use
standard C++ or base types instead. Additionally, do not place foundational math
routines in higher layers like src/core/ to bypass dependency limits on
lightweight types; forward-declare trivial types (such as struct SkPoint;) or
move them to base to keep utilities factored in base modules.

**Why.** src/base/ and include/private/base/ form Skia's foundational layer upon
which core and other subsystems depend. Including core headers introduces
circular architectural dependencies and violates build-time layering checks.
Conversely, moving foundational utilities up into core to access types fragments
related math algorithms and degrades API typing; forward-declaring or migrating
trivial types maintains clean layering and strong architectural cohesion.

## Examples

Bad:

```cpp
// In src/base/SkAutoLocaleSetter.h:
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"

// In src/base/SkCubics.cpp:
#include "include/core/SkScalar.h"
if (!SkScalarIsFinite(R2MinusQ3)) { ... }

// In src/core/SkGeometry.h to avoid src/base depending on SkPoint.h:
class SkBezierQuad {
public:
    static SkSpan<const SkScalar> IntersectWithHorizontalLine(
            SkSpan<const SkPoint> controlPoints, SkScalar yIntercept,
            SkScalar storage[2]);
};
```

Good:

```cpp
// In src/base/SkAutoLocaleSetter.h:
#include "include/private/base/SkNoncopyable.h"
#include <optional>
#include <string>

// In src/base/SkCubics.cpp:
#include "include/private/base/SkFloatingPoint.h"
if (!SkIsFinite(R2MinusQ3)) { ... }

// In src/base/SkBezierCurves.h:
struct SkPoint;
class SkBezierQuad {
public:
    static SkSpan<const float> IntersectWithHorizontalLine(
            SkSpan<const SkPoint> controlPoints, float yIntercept,
            float storage[2]);
};
```

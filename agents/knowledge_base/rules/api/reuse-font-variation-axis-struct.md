---
name: skia/api/reuse-font-variation-axis-struct
description: When representing font variation axes in font scanner or typeface APIs, reuse SkFontParameters::Variation::Axis with float bounds instead of ad-hoc structs or SkFixed
rubric: correctness
severity: should_fix
---

**Rule.** When defining or updating font scanning, font manager, or typeface
APIs that query or pass font variation axes, reuse the canonical
SkFontParameters::Variation::Axis struct with float bounds. Do not declare
duplicate, parallel, or ad-hoc axis structs (such as custom AxisDefinition
types) or use fixed-point types (SkFixed) to represent font variation
parameters.

**Why.** Skia defines SkFontParameters::Variation::Axis as the standard
representation for font variation axes across core font APIs and backend
implementations. FreeType historically reported axis values in 16.16 fixed point
(SkFixed), but platform backends like DirectWrite, CoreText, and Fontations only
work with floating-point values. Using SkFixed or custom axis structs causes
type proliferation, introduces precision loss, and prevents clean interoperation
with float-based font engines.

## Examples

Bad:

```cpp
struct AxisDefinition {
    SkFourByteTag fTag;
    SkFixed fMinimum;
    SkFixed fDefault;
    SkFixed fMaximum;
};
using AxisDefinitions = skia_private::STArray<4, AxisDefinition, true>;

virtual bool scanAxes(SkStream*, AxisDefinitions* axes) const = 0;
```

Good:

```cpp
#include "include/core/SkFontParameters.h"

using AxisDefinitions = skia_private::STArray<4, SkFontParameters::Variation::Axis, true>;

virtual bool scanAxes(SkStream*, AxisDefinitions* axes) const = 0;
```

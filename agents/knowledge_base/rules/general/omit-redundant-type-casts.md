---
name: skia/omit-redundant-type-casts
description: When expressions match target types, enums are compared, operands promote, or values are bounds-checked, omit redundant casts and SkToInt or SkIntToFloat helpers
rubric: readability
severity: should_fix
---

**Rule.** Do not write redundant type casts (reinterpret_cast, static_cast, or
C-style) or conversion helpers (such as SkToInt or SkIntToFloat) when an
expression or pointer already matches the target type, target types differ, or
values are already bounds-checked. In binary arithmetic and comparisons, omit
redundant widening or float conversion casts where C++ promotions apply, and use
float literal divisors (e.g., / 255.0f) rather than casting the dividend.
Likewise, do not cast enum values to integer types (such as (int) or
static_cast<int>) when comparing values of the same enum, as enums natively
support comparison operators.

**Why.** Redundant casts and conversion helpers add visual noise without
altering program semantics. When applied to matching types, non-matching
targets, or checked bounds, they mislead readers about conversions or bounds
checking; in mixed arithmetic, C++ promotion rules apply automatically.
Similarly, casting enums to integer types for comparisons bypasses enum type
safety and adds clutter when native comparison operators are already
well-defined.

## Examples

Bad:

```cpp
// Redundant reinterpret_cast or static_cast on already typed pointers and expressions
SkPathVerbAnalysis analysis = SkPathPriv::AnalyzeVerbs({
    reinterpret_cast<const SkPathVerb*>(verbs), counts.vbs
});
switch (v = (SkPathVerb)fVerbs[vIndex++]) {
    case SkPathVerb::kMove: break;
}
SkColor color = static_cast<SkColor>(paint.getColor());

// Mismatched or redundant SkToInt when target type is uint16_t or bounds are checked
uint16_t countData(uint16_t a, uint16_t b) {
    SkASSERT_RELEASE((uint32_t)a + (uint32_t)b + 1 <= std::numeric_limits<uint16_t>::max());
    return SkToInt(a + b + 1);
}

// Redundant widening or float conversion in binary arithmetic and comparisons
if (SkLeftShift((int64_t)SkAbs32(dy), 6) > (int64_t)SkAbs32(dx)) { ... }
const float left = std::max(UL.fX, LL.fX) + SkIntToFloat(2 * margin.fX);

// Casting dividend to float instead of using a float literal divisor
float ambientf = static_cast<float>(fLight.fAmbient) / 255;

// Casting enums to int for relational comparison
if (finalBlendMode.has_value() &&
    (int)finalBlendMode.value() > (int)SkBlendMode::kLastCoeffMode) { ... }
```

Good:

```cpp
// Pass typed pointers and expressions directly without redundant casts
SkPathVerbAnalysis analysis = SkPathPriv::AnalyzeVerbs({verbs, counts.vbs});
switch (v = fVerbs[vIndex++]) {
    case SkPathVerb::kMove: break;
}
SkColor color = paint.getColor();

// Return expression directly when already bounds-checked and matching target type
uint16_t countData(uint16_t a, uint16_t b) {
    SkASSERT_RELEASE((uint32_t)a + (uint32_t)b + 1 <= std::numeric_limits<uint16_t>::max());
    return a + b + 1;
}

// Operands promote automatically in mixed arithmetic and comparisons
if (SkLeftShift((int64_t)SkAbs32(dy), 6) > SkAbs32(dx)) { ... }
const float left = std::max(UL.fX, LL.fX) + 2 * margin.fX;

// Float literal divisor promotes division to float without static_cast
float ambientf = fLight.fAmbient / 255.0f;

// Compare enums directly using native relational operators
if (finalBlendMode.has_value() &&
    finalBlendMode.value() > SkBlendMode::kLastCoeffMode) { ... }
```

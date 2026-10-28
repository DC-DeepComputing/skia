---
name: skia/omit-redundant-role-suffixes-from-parameters
description: When naming function parameters or local variables, omit redundant role suffixes like 'Param' or 'Arg' and primitive-type prefixes; use descriptive domain names
rubric: readability
severity: nit
---

**Rule.** Do not append redundant role suffixes (such as 'Param' or 'Arg') or
prepend primitive-type prefixes (such as 'int' or 'float' Hungarian notation,
e.g. 'intStops' instead of 'numStops') to parameter or variable names. Name
identifiers concisely and descriptively after their domain concept, keeping
naming idiomatic and avoiding transient refactoring renames.

**Why.** Type prefixes and role suffixes add visual clutter without conveying
useful semantics, as types and parameter roles are already evident from
declaration syntax. Descriptive domain identifiers clarify intent, prevent
unnecessary signature churn during refactoring, and maintain consistency with
Skia naming conventions.

## Examples

Bad:

```cpp
// Redundant role suffixes ('Param', 'Arg') and primitive type prefixes ('int', 'float'):
$pure float2 $conical_grad_layout(float radius0Param,
                                  float dRadius,
                                  float a,
                                  float invA,
                                  float2 pos);

half4 $colorize_grad_tex(sampler2D colorsAndOffsetsSampler, int intStops, float2 t);

void processBounds(const SkRect& boundsParam, float floatScale);
```

Good:

```cpp
// Concise parameter names reflecting the domain concept without role suffixes or type prefixes:
$pure float2 $conical_grad_layout(float radius0,
                                  float dRadius,
                                  float a,
                                  float invA,
                                  float2 pos);

half4 $colorize_grad_tex(sampler2D colorsAndOffsetsSampler, int numStops, float2 t);

void processBounds(const SkRect& bounds, float scaleFactor);
```

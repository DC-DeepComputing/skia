---
name: skia/prefer-west-const-over-east-const
description: When qualifying variables, parameters, or return types as const, place const before the type or auto (west-const) rather than after (east-const)
rubric: readability
severity: nit
---

**Rule.** Place the const qualifier before the type or auto specifier
(west-const, e.g., const auto*, const auto&, const Type*, const Type&) rather
than after it (east-const, e.g., auto const*, Type const&). Apply this placement
consistently across local variable declarations, function parameters, return
types, and member fields.

**Why.** Skia standardizes on west-const throughout the repository. Adhering to
const auto* and const Type& ensures visual consistency across all translation
units, matches codebase idioms, and avoids mixing east-const and west-const
styles within the same project.

## Examples

Bad:

```cpp
auto const* globalCache =
    this->resourceProvider()->sharedContext()->globalCache();
void processBuffer(GrBuffer const* buffer, SkSpan<int const> values);
```

Good:

```cpp
const auto* globalCache =
    this->resourceProvider()->sharedContext()->globalCache();
void processBuffer(const GrBuffer* buffer, SkSpan<const int> values);
```

---
name: skia/sksl/prefer-modifierflags-accessors
description: When inspecting SkSL ModifierFlags on variables or declarations, call is<Flag>() helper methods rather than bitwise masking with ModifierFlag::k
rubric: readability
severity: nit
---

**Rule.** When checking modifier flags on SkSL variables, functions, or
declarations, call ModifierFlags predicate accessor methods (such as
flags.isFlat(), flags.isConst(), flags.isUniform(), or flags.isPure()) rather
than raw bitwise masking (flags & ModifierFlag::k...). If a needed modifier
query method is missing, add the inline is<Flag>() helper method to
SkSL::ModifierFlags in src/sksl/ir/SkSLModifierFlags.h.

**Why.** ModifierFlags wraps SkEnumBitMask<ModifierFlag> and encapsulates flag
inspection in self-documenting boolean predicate methods. Using dedicated
accessors improves readability, avoids repetitive SkToBool conversions and
bitwise masking boilerplate at call sites, and maintains consistent flag query
idioms across SkSL IR analysis and backend code generators.

## Examples

Bad:

```cpp
// Inspecting modifier flags using raw bitwise masking
if ((flags & ModifierFlag::kFlat) ||
    type.isInteger() || (type.isVector() && type.componentType().isInteger())) {
    return "@interpolate(flat)";
}
```

Good:

```cpp
// Inspecting modifier flags using dedicated accessor methods
if (flags.isFlat() ||
    type.isInteger() || (type.isVector() && type.componentType().isInteger())) {
    return "@interpolate(flat)";
}
```

---
name: skia/sksl/prefer-extended-over-size-descriptors-for-subclasses
description: When naming subclasses holding auxiliary fields beyond a base type, use descriptive qualifiers like Extended instead of colloquial size descriptors like Big
rubric: readability
severity: nit
---

**Rule.** When creating a subclass or variant that holds uncommon or auxiliary
fields beyond the base type, prefer descriptive names like Extended<Type> over
colloquial size descriptors like Big<Type>.

**Why.** Colloquial size adjectives like 'Big' describe memory footprint
informally rather than conveying the semantic intent of the type. Using
descriptive prefixes such as 'Extended' clearly communicates that the class
augments the base type with supplementary fields or capabilities.

## Examples

Bad:

```cpp
class BigVariable final : public Variable {
    ...
};
```

Good:

```cpp
class ExtendedVariable final : public Variable {
    ...
};
```

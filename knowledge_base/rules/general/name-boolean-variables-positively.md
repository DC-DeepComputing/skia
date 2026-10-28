---
name: skia/name-boolean-variables-positively
description: When declaring boolean variables, members, or predicates, name them positively using affirmative conditions rather than negative phrasing like doesNot or no
rubric: readability
severity: nit
---

**Rule.** Name boolean variables, member flags, and predicate functions
positively using affirmative conditions (such as needsFoo, hasBar, or isValid)
rather than negative phrasing (such as doesNotNeedFoo, hasNoBar, or isInvalid).
Express conditions positively so downstream logic does not encounter confusing
double negatives when negating or combining expressions.

**Why.** Negative boolean names lead to awkward, counter-intuitive double
negatives (such as !doesNotNeedFoo or !hasNoBar) in conditional checks and
compound logic. Phrasing boolean identifiers positively reduces cognitive load
and keeps conditional branching clear and maintainable.

## Examples

Bad:

```cpp
const bool swizzleDoesNotNeedGamutTransform =
        data.fReadSwizzle == ReadSwizzle::kRGBA || data.fReadSwizzle == ReadSwizzle::kRGB1;
if (!swizzleDoesNotNeedGamutTransform) {
    applyGamutTransform();
}
```

Good:

```cpp
const bool swizzleNeedsGamutTransform = !(data.fReadSwizzle == ReadSwizzle::kRGBA ||
                                          data.fReadSwizzle == ReadSwizzle::kRGB1);
if (swizzleNeedsGamutTransform) {
    applyGamutTransform();
}
```

---
name: skia/api/prefix-non-mutating-methods-with-make
description: When declaring methods that return a new transformed or derived copy of an object, prefix the name with make rather than bare verbs
rubric: readability
severity: should_fix
---

**Rule.** When declaring methods that return a new transformed or derived copy
of an object (such as applying a matrix, offset, or configuration adjustment),
prefix the method name with make (e.g., makeTransform(), makeOffset()). Do not
use bare verb names (e.g., transform(), offset()) for non-mutating methods that
return a new instance.

**Why.** In Skia conventions, bare verb method names conventionally perform
in-place mutation on the object or write to an out-parameter. Prefixing
non-mutating transformation and factory methods with make clarifies at
declaration and call sites that the original object remains unmodified and a
newly allocated or derived instance is returned.

## Examples

Bad:

```cpp
class SkPathData {
public:
    sk_sp<SkPathData> transform(const SkMatrix&) const;
    sk_sp<SkPathData> offset(SkVector) const;
};
```

Good:

```cpp
class SkPathData {
public:
    sk_sp<SkPathData> makeTransform(const SkMatrix&) const;
    sk_sp<SkPathData> makeOffset(SkVector) const;
};
```

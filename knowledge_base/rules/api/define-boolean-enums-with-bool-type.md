---
name: skia/api/define-boolean-enums-with-bool-type
description: 'When defining two-state boolean enum classes, declare them with an explicit '': bool'' underlying type and ''kNo = false, kYes = true'' enumerators'
rubric: readability
severity: should_fix
---

**Rule.** Define two-state (boolean) scoped enums with an explicit underlying
type of ': bool' and explicit values 'kNo = false, kYes = true'. Do not omit the
underlying ': bool' type or leave enumerator values implicit when representing
binary yes/no states.

**Why.** Specifying ': bool' guarantees the enum is stored as a single byte
rather than defaulting to 'int', conserving memory in structs and arrays.
Explicitly defining 'kNo = false, kYes = true' establishes a consistent,
self-documenting convention across Skia and ensures safe, predictable
conversions between boolean expressions and enum states.

## Examples

Bad:

```cpp
enum class SkResolveConvexity {
    kNo,
    kYes,
};
```

Good:

```cpp
enum class SkResolveConvexity : bool {
    kNo = false,
    kYes = true,
};
```

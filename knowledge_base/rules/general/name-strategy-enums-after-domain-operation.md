---
name: skia/name-strategy-enums-after-domain-operation
description: When defining optimization, strategy, or dispatch enums with operation-level variants, name the enum type after the overarching operation rather than narrow sub-concepts
rubric: readability
severity: nit
---

**Rule.** Name optimization, dispatch, or strategy enum types after the
overarching operation or domain (e.g., SkBlendFastPath) rather than an overly
specific sub-concept or attribute (e.g., SkBlendModeFastPath) when enumerator
variants represent broader operation-level actions (such as skipping execution
or drawing entirely). Ensure the type name accurately encompasses all possible
variant outcomes without falsely constraining the enum's conceptual scope.

**Why.** Embedding narrow sub-concept qualifiers into an enum type's name is
misleading when some variants encompass actions outside that sub-concept (such
as kSkipDrawing, which aborts the entire draw/blend operation rather than
selecting a blend mode). Using concise, domain-level names accurately reflects
the full scope of outcomes and simplifies call-site dispatch.

## Examples

Bad:

```cpp
// Bad: 'Mode' qualifier is overly narrow since 'kSkipDrawing' bypasses the entire draw/blend operation.
enum class SkBlendModeFastPath {
    kNormal,
    kSrcOver,
    kSkipDrawing,
};

SkBlendModeFastPath CheckFastPath(const SkPaint& paint, bool dstIsOpaque);
```

Good:

```cpp
// Good: 'SkBlendFastPath' accurately encompasses both blend mode selection and operation bypass.
enum class SkBlendFastPath {
    kNormal,
    kSrcOver,
    kSkipDrawing,
};

SkBlendFastPath CheckFastPath(const SkPaint& paint, bool dstIsOpaque);
```

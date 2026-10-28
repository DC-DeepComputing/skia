---
name: skia/encapsulate-type-tagged-downcast-helpers
description: When downcasting base classes that use runtime type tags, encapsulate the null check, tag validation, and static_cast in dedicated helper functions
rubric: design
severity: should_fix
---

**Rule.** Encapsulate downcasting of base objects that rely on runtime type tags
or enums into dedicated helper functions (e.g., AsGraphiteRecorder(),
AsGaneshRecorder()) that handle null checking, type validation, and static_cast.
Do not duplicate manual type checks and static_cast calls across multiple call
sites.

**Why.** In class hierarchies where RTTI is disabled and base classes use
runtime type discriminators, open-coding null checks, type comparisons, and
static_cast across callers produces repetitive boilerplate and risks unchecked
or unsafe downcasts. Centralizing the downcast in a dedicated helper provides a
concise, safe conversion that returns nullptr on null or type mismatch.

## Examples

Bad:

```cpp
if (!recorder || recorder->type() != SkRecorder::Type::kGraphite) {
    return nullptr;
}
return SkSurfaces::RenderTarget(static_cast<Recorder*>(recorder), info);
```

Good:

```cpp
auto gRecorder = AsGraphiteRecorder(recorder);
if (!gRecorder) {
    return nullptr;
}
return SkSurfaces::RenderTarget(gRecorder, info);
```

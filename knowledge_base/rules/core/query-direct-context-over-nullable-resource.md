---
name: skia/core/query-direct-context-over-nullable-resource
description: When querying context or backend properties on filter or pipeline objects, query directly held context state instead of dereferencing nullable child resources
rubric: correctness
severity: blocking
---

**Rule.** Do not traverse or dereference child resources or inputs (such as
fSource.image()) that may be null to inspect backend capabilities or retrieve
context pointers. Query the enclosing class's directly held context or backend
state (such as fGaneshContext) directly, or explicitly check for null before
accessing child properties.

**Why.** Child resources or filter inputs may be omitted, empty, or
uninitialized, returning null and causing undefined behavior or crashes when
unconditionally dereferenced. Querying directly held context pointers is safe
and avoids coupling backend inspection to child resource presence.

## Examples

Bad:

```cpp
// ContextInfo::fSource can contain a null image
bool gpuBacked() const { return fInfo.fSource.image()->isTextureBacked(); }
GrRecordingContext* getContext() const { return fInfo.fSource.image()->getContext(); }
```

Good:

```cpp
// Query the directly held context instead of traversing the nullable source
bool gpuBacked() const { return SkToBool(fGaneshContext); }
GrRecordingContext* getContext() const { return fGaneshContext; }
```

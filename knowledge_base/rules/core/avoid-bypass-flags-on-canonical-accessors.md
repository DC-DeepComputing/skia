---
name: skia/core/avoid-bypass-flags-on-canonical-accessors
description: When callers require a distinct instance of a singleton type, invoke constructors or factories directly instead of adding bypass flags to Empty() accessors
rubric: design
severity: nit
---

**Rule.** Do not add ad-hoc boolean configuration flags (such as
forceNewInstance) to canonical or singleton accessors like Empty() to serve
callers needing distinct instances. Call the underlying constructor or factory
method directly at call sites requiring a unique instance.

**Why.** Complicating a canonical accessor's signature with bypass flags to DRY
up a single call site introduces unnecessary configurability and bloats shared
APIs. Keeping accessors parameterless ensures predictable singleton semantics,
while callers needing unique instances should instantiate them explicitly.

## Examples

Bad:

```cpp
// In SkPathData.h:
static sk_sp<SkPathData> Empty(bool forceNewInstance = false);

// In caller requiring a distinct empty sentinel:
static SkPathData* gErrorSingleton = SkPathData::Empty(true).release();
```

Good:

```cpp
// In SkPathData.h:
static sk_sp<SkPathData> Empty();

// In caller requiring a distinct empty sentinel:
static SkPathData* gErrorSingleton = SkPathData::MakeNoCheck({}, {}, {}, {}, {}).release();
```

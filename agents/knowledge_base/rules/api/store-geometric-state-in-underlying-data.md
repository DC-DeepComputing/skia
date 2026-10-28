---
name: skia/api/store-geometric-state-in-underlying-data
description: When adding geometric or validity properties to lightweight wrapper classes like SkPath, store state in underlying data objects rather than wrapper members
rubric: design
severity: should_fix
---

**Rule.** Do not store geometric or data-dependent properties (such as
finiteness or validity) on high-level wrapper objects like SkPath. Keep
intrinsic geometry state on or derived from the underlying data object (such as
SkPathData or SkPathRef), using dedicated sentinel data singletons to represent
error or invalid states if needed.

**Why.** Wrapper types like SkPath are lightweight value handles referencing
shared, copy-on-write underlying geometry data. Storing geometry-dependent
properties in the wrapper bloats object size, requires redundant state
synchronization across mutating methods, and risks inconsistencies where
multiple wrappers sharing the same data disagree.

## Examples

Bad:

```cpp
class SkPath {
    sk_sp<SkPathData> fPathData;
    SkPathFillType    fFillType;
    bool              fIsVolatile;
    bool              fIsFinite; // Geometric property stored in wrapper
};
```

Good:

```cpp
class SkPath {
    sk_sp<SkPathData> fPathData;
    SkPathFillType    fFillType;
    bool              fIsVolatile;

    bool isFinite() const {
        return fPathData.get() != PeekErrorSingleton();
    }
    static SkPathData* PeekErrorSingleton();
};
```

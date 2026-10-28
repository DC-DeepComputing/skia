---
name: skia/prefer-skpath-over-builder-for-member-storage
description: When storing path geometry as class or struct members that are not modified after initialization, store SkPath instead of SkPathBuilder
rubric: design
severity: should_fix
---

**Rule.** Do not use SkPathBuilder for class or struct member storage when the
path geometry is immutable and neither edited nor passed to builder-only APIs;
store SkPath (or an optional/lazy wrapper like std::optional<SkPath> or
SkTLazy<SkPath>) instead.

**Why.** SkPathBuilder is designed for constructing and mutating paths, whereas
SkPath represents immutable geometry and has lightweight copy-on-write
semantics. Storing SkPathBuilder in classes or structs that only inspect or
query path geometry introduces unnecessary abstraction, prevents returning const
SkPath& or caching bounds, and forces callers to call snapshot() to obtain an
SkPath.

## Examples

Bad:

```cpp
class Element {
public:
    SkPath getDeviceSpacePath() const {
        return fDeviceSpacePath.snapshot();
    }
private:
    SkPathBuilder fDeviceSpacePath;
};
```

Good:

```cpp
class Element {
public:
    const SkPath& getDeviceSpacePath() const {
        return *fDeviceSpacePath;
    }
private:
    std::optional<SkPath> fDeviceSpacePath;
};
```

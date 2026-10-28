---
name: skia/avoid-skpath-snapshot
description: When copying or returning an SkPath instance, pass or return it directly by value instead of calling redundant SkPath::snapshot()
rubric: readability
severity: should_fix
---

**Rule.** Do not call snapshot() on an SkPath instance when copying, passing, or
returning a path. SkPath has copy-on-write value semantics, so copy or return
SkPath directly by value rather than invoking redundant member methods.

**Why.** SkPath is a lightweight copy-on-write object holding an sk_sp handle to
underlying geometry data; its copy constructor and assignment operators share
data efficiently without deep copying. Calling snapshot() on an SkPath is a
redundant no-op that merely returns *this, confusing SkPath with SkPathBuilder.

## Examples

Bad:

```cpp
SkPath CopyPath(const SkPath& path) {
    return path.snapshot();
}
```

Good:

```cpp
SkPath CopyPath(SkPath path) {
    return path;
}
```

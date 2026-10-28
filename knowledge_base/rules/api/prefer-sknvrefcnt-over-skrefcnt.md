---
name: skia/api/prefer-sknvrefcnt-over-skrefcnt
description: When defining reference-counted classes without virtual methods, inherit from SkNVRefCnt<T> instead of SkRefCnt and mark the class final
rubric: safety
severity: should_fix
---

**Rule.** Inherit from SkNVRefCnt<T> instead of SkRefCnt for reference-counted
classes that do not require virtual methods, and mark the derived class final.
If the destructor is protected or private, declare 'friend class SkNVRefCnt<T>;'
to allow CRTP non-virtual deletion.

**Why.** SkRefCnt derives from SkRefCntBase, which defines a virtual destructor
that adds a vtable pointer (vptr) and dynamic dispatch overhead to every
instance. SkNVRefCnt<T> provides non-virtual reference counting via CRTP and
deletes through static_cast<const T*>(this), eliminating vptr memory overhead
but requiring classes to be final to avoid slicing or undefined behavior during
polymorphic destruction.

## Examples

Bad:

```cpp
// Adds 8-byte vptr overhead despite having no virtual methods
class SkPathData : public SkRefCnt {
public:
    ...
};
```

Good:

```cpp
// Eliminates vptr overhead via non-virtual CRTP ref-counting
class SkPathData final : public SkNVRefCnt<SkPathData> {
public:
    ...
private:
    ~SkPathData() = default;
    friend class SkNVRefCnt<SkPathData>;
};
```

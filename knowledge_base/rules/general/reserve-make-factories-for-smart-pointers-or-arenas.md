---
name: skia/reserve-make-factories-for-smart-pointers-or-arenas
description: When defining classes, reserve static Make factories for smart pointers or arena allocation; use public constructors instead of returning raw heap pointers from Make
rubric: readability
severity: nit
---

**Rule.** Reserve static Make factory methods for types managed by smart
pointers (such as sk_sp or std::unique_ptr), allocated in arenas (such as
SkArenaAlloc), or fallible creation returning nullptr. Do not return raw
heap-allocated pointers from static Make factories for internally managed
objects; provide and use public constructors instead.

**Why.** In Skia conventions, Make factory methods communicate smart pointer
ownership, arena lifecycle management, or fallible initialization. Wrapping
plain heap allocation in a static Make factory returning a raw pointer is
redundant, obscures standard C++ object construction, and violates the
convention that raw pointers represent borrowed references.

## Examples

Bad:

```cpp
class IntrinsicBuffer final {
public:
    static IntrinsicBuffer* Make(sk_sp<DawnBuffer> dawnBuffer) {
        return new IntrinsicBuffer(std::move(dawnBuffer));
    }
private:
    IntrinsicBuffer(sk_sp<DawnBuffer> dawnBuffer) : fDawnBuffer(std::move(dawnBuffer)) {}
};
```

Good:

```cpp
class IntrinsicBuffer final {
public:
    IntrinsicBuffer(sk_sp<DawnBuffer> dawnBuffer) : fDawnBuffer(std::move(dawnBuffer)) {}
};
```

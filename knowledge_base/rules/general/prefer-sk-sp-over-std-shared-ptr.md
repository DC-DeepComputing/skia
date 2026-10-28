---
name: skia/prefer-sk-sp-over-std-shared-ptr
description: When designing or passing objects with shared ownership across APIs, use SkRefCnt and sk_sp rather than std::shared_ptr
rubric: readability
severity: should_fix
---

**Rule.** Use SkRefCnt (or SkNVRefCnt) and sk_sp<T> (with factory functions
returning sk_sp<T>) rather than std::shared_ptr<T> for objects intended to be
passed by shared reference across APIs. Do not use std::make_shared,
std::weak_ptr, or std::enable_shared_from_this.

**Why.** Skia standardizes on intrusive reference counting via SkRefCnt and
sk_sp for shared ownership across public and internal interfaces.
std::shared_ptr relies on extrinsic control blocks that add extra allocation and
indirection overhead, and is banned in the Chromium C++ style guide that Skia
follows.

## Examples

Bad:

```cpp
// Bad: Using std::shared_ptr across class interfaces
class SK_API SkUnicode {
    virtual std::unique_ptr<SkUnicode> copy() = 0;
};

void ProcessUnicode(std::shared_ptr<SkUnicode> unicode);
```

Good:

```cpp
// Good: Inheriting SkRefCnt and passing shared ownership via sk_sp
class SK_API SkUnicode : public SkRefCnt {
    virtual sk_sp<SkUnicode> copy() = 0;
};

void ProcessUnicode(sk_sp<SkUnicode> unicode);
```

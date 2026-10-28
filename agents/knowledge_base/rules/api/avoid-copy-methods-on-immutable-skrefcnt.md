---
name: skia/api/avoid-copy-methods-on-immutable-skrefcnt
description: When designing immutable reference-counted classes derived from SkRefCnt, share instances via sk_sp instead of providing copy() or cloning methods
rubric: safety
severity: should_fix
---

**Rule.** Do not provide copy() or cloning methods on immutable
reference-counted classes (derived from SkRefCnt or SkNVRefCnt); share instances
via sk_sp instead. Callers requiring shared access should retain or pass the
existing sk_sp handle rather than duplicating identical heap objects.

**Why.** Immutable reference-counted objects contain no externally mutable
state, making deep copies behaviorally indistinguishable from the original
instance. Introducing copy() or clone() methods encourages redundant heap
allocations, unnecessary object construction, and atomic reference-count
overhead where sharing ownership via sk_sp is thread-safe and zero-allocation.

## Examples

Bad:

```cpp
class SkUnicode : public SkRefCnt {
public:
    // Anti-pattern: cloning an immutable ref-counted class
    virtual sk_sp<SkUnicode> copy() = 0;
    virtual bool isControl(SkUnichar utf8) = 0;
};
```

Good:

```cpp
class SkUnicode : public SkRefCnt {
public:
    // Immutable ref-counted instances are shared via sk_sp; no copy() needed.
    virtual bool isControl(SkUnichar utf8) = 0;
};

// At call sites, retain or pass the sk_sp handle:
sk_sp<SkUnicode> unicode = getUnicode();
sk_sp<SkUnicode> shared = unicode; // Shares reference without allocating
```

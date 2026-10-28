---
name: skia/avoid-noop-subclass-hierarchies
description: When designing classes for optional or toggleable features, avoid introducing intermediate abstract hierarchies and no-op stub subclasses when concrete classes or nullptr checks suffice
rubric: design
severity: should_fix
---

**Rule.** Do not introduce multi-tier class hierarchies, intermediate abstract
classes, or no-op stub subclasses merely to support disabling or bypassing a
feature. Before introducing virtual dispatch layers and no-op stubs, evaluate
whether enabling the functionality unconditionally, keeping the class concrete,
or representing the disabled state with a nullptr check provides a simpler, more
direct design.

**Why.** Multi-tier hierarchies with intermediate abstract classes and no-op
stub implementations introduce unnecessary vtable indirection, dynamic
allocation overhead, and maintenance complexity. Using concrete classes or
simple nullptr checks keeps class architectures flat, avoids redundant
boilerplate subclasses, and eliminates speculative abstraction layers.

## Examples

Bad:

```cpp
// Bad: Multi-tier hierarchy with virtual methods and no-op subclass to disable tracing
class DebugTracePriv : public DebugTrace {
public:
    virtual void setTraceCoord(const SkIPoint& coord) {}
    virtual bool readTrace(SkStream* r) { return false; }
    static sk_sp<DebugTracePriv> MakeNoOp();
};
class DebugTraceImpl : public DebugTracePriv { ... };
```

Good:

```cpp
// Good: Flat concrete class when feature is always enabled or guarded by nullptr
class DebugTracePriv : public DebugTrace {
public:
    void setTraceCoord(const SkIPoint& coord);
    bool readTrace(SkStream* r);
};
```

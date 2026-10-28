---
name: skia/api/name-stateful-placeholders-as-stub-not-noop
description: When declaring factory methods or placeholder classes that retain state or data, name them using Stub rather than NoOp identifiers like MakeNoOp
rubric: readability
severity: nit
---

**Rule.** Do not name factory methods or placeholder implementations with 'NoOp'
(such as MakeNoOp()) if the returned object retains state, stores data, or
produces side effects. Name stateful placeholder implementations using 'Stub'
(such as MakeStub()), reserving 'NoOp' strictly for stateless implementations
that discard all operations without side effects.

**Why.** The term 'NoOp' denotes an operation that performs no action and
discards inputs without observable state changes or storage. When an object
accepts, buffers, or retains data structures—even if it omits downstream
execution or heavy processing—labeling it 'NoOp' misleads callers about its
lifecycle and side effects, whereas 'Stub' accurately conveys a lightweight
placeholder.

## Examples

Bad:

```cpp
// Bad: Factory returns an object that retains state/data, but is named MakeNoOp
class DebugTracePriv : public DebugTrace {
public:
    static sk_sp<DebugTracePriv> MakeNoOp();
    void setTraceCoord(const SkIPoint& coord);
    bool readTrace(SkStream* r);
};
```

Good:

```cpp
// Good: Stateful placeholder factory is named MakeStub; NoOp is reserved for stateless objects
class DebugTracePriv : public DebugTrace {
public:
    static sk_sp<DebugTracePriv> MakeStub();
    void setTraceCoord(const SkIPoint& coord);
    bool readTrace(SkStream* r);
};
```

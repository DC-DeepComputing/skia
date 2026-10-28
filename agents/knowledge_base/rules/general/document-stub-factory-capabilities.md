---
name: skia/document-stub-factory-capabilities
description: When declaring stub, mock, or reduced-functionality factory methods, document in header comments what behaviors or storage they support versus what logic is omitted
rubric: readability
severity: nit
---

**Rule.** When declaring stub, mock, or reduced-functionality factory methods
(such as MakeStub, MakeNoOp, or MakeNull), document in a doc comment what
capabilities, storage, or behaviors the returned instance supports versus what
operations or calculations are omitted. Clarify whether the instance safely
accepts and retains data, silently discards operations, or returns dummy values
to prevent invalid caller assumptions.

**Why.** Callers cannot deduce from method signatures or names alone whether a
stub safely accepts data, silently discards operations, or returns dummy values.
Explicitly documenting what capabilities are supported versus omitted prevents
callers from making invalid assumptions about side effects, processing, or data
retention.

## Examples

Bad:

```cpp
class DebugTracePriv {
public:
    // Bad: Undocumented stub factory leaves behavior and data retention ambiguous.
    static sk_sp<DebugTracePriv> MakeStub();
};
```

Good:

```cpp
class DebugTracePriv {
public:
    /**
     * Returns an implementation which can have data stored to it, but has no logic for
     * writing traces or doing other calculations.
     */
    static sk_sp<DebugTracePriv> MakeStub();
};
```

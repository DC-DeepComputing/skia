---
name: skia/name-boolean-flags-as-predicates-rather-than-modes
description: When naming boolean member flags representing data source, provenance, or conditions, name them as predicates rather than using Mode suffixes
rubric: readability
severity: nit
---

**Rule.** Name boolean member flags after the concrete condition, origin, or
provenance they represent (such as fIsFromIco) using predicate phrasing. Do not
suffix boolean flags with Mode (such as fIsIcoMode) when they merely track data
origin or a binary condition rather than a configurable operational state
machine.

**Why.** A 'Mode' suffix implies an operational state machine, execution policy,
or multi-state configuration. Using 'Mode' for a simple boolean provenance or
condition flag misleads readers about the object's lifecycle and state space,
whereas predicate naming makes its boolean nature and intent self-documenting.

## Examples

Bad:

```cpp
// Bad: 'Mode' suffix falsely suggests an operational state machine or configurable mode.
class SkBmpRustCodec : public SkCodec {
    ...
    const bool fIsIcoMode;
};
```

Good:

```cpp
// Good: predicate phrasing clearly communicates data provenance and boolean condition.
class SkBmpRustCodec : public SkCodec {
    ...
    const bool fIsFromIco;
};
```

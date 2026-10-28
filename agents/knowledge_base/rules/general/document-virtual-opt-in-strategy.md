---
name: skia/document-virtual-opt-in-strategy
description: When documenting virtual opt-in or capability query methods, state the high-level rendering strategy toggled rather than whether callers can invoke helpers
rubric: readability
severity: nit
---

**Rule.** In doc comments for virtual opt-in or capability query methods,
describe the high-level rendering behavior or strategy being toggled rather than
phrasing the query around whether callers 'can' invoke underlying helpers.
Additionally, if opting into the behavior imposes specific constraints or
guarantees on related virtual methods (such as restricted arguments or format
invariants), document them explicitly as caller invariants.

**Why.** Phrasing capability queries around whether a caller 'can' call a helper
makes it ambiguous whether the method is an optional capability probe or a
mandatory precondition. Describing the high-level behavioral strategy and
explicitly documenting constraints on related methods prevents confusion for
derived class implementers.

## Examples

Bad:

```cpp
// TEMPORARY: Whether or not SkCanvas can call createDevice() with an alpha-only color type
// and use image filters to simulate mask filters and then draw the filtered mask using
// drawCoverageMask.
virtual bool useDrawCoverageMaskForMaskFilters() const { return false; }
```

Good:

```cpp
// TEMPORARY: Whether or not SkCanvas should use a layer and image filters to simulate
// mask filters and then draw the filtered mask using drawCoverageMask. Unlike regular
// layers, the color type passed to SkDevice::createDevice() will always be an alpha-only
// color type.
virtual bool useDrawCoverageMaskForMaskFilters() const { return false; }
```

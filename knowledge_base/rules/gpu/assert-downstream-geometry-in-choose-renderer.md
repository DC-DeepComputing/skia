---
name: skia/gpu/assert-downstream-geometry-in-choose-renderer
description: When dispatching renderers in Device::chooseRenderer(), assert against downstream-only intermediate geometry types with SkASSERT instead of adding speculative dead branches
rubric: readability
severity: should_fix
---

**Rule.** In Device::chooseRenderer() and top-level draw dispatchers, do not add
dead or speculative dispatch branches for intermediate geometry types that are
generated in-situ downstream (such as within drawGeometry()). Enforce that
callers only supply valid top-level geometry by asserting against
downstream-only types (e.g., using SkASSERT(!geometry.isEndCaps() &&
!geometry.isWideTiles())).

**Why.** Speculatively adding dispatch cases for geometry types produced only
downstream introduces unreachable dead code and misleads maintainers into
believing those sub-geometries can be dispatched directly. Asserting at the
dispatcher entry point documents and enforces pipeline layering invariants while
catching unexpected call patterns early.

## Examples

Bad:

```cpp
// In Device::chooseRenderer:
#if defined(SK_ENABLE_SPARSE_STRIPS)
} else if (geometry.isWideTiles()) {
    return {renderers->sparseStripsWideTile(), nullptr};
} else if (geometry.isEndCaps()) {
    return {renderers->sparseStripsEndCap(), nullptr};
#endif
```

Good:

```cpp
// In Device::chooseRenderer:
#if defined(SK_ENABLE_SPARSE_STRIPS)
    // Intermediate sparse strips geometry is produced in-situ downstream in drawGeometry.
    SkASSERT(!geometry.isEndCaps() && !geometry.isWideTiles());
#endif
```

---
name: skia/name-multi-entity-predicates-with-plural-phrasing
description: When naming predicate or query functions that evaluate multiple entities or return compound boolean results, use plural phrasing (e.g., are_axes or AreFinite)
rubric: readability
severity: nit
---

**Rule.** When naming predicate or query functions that evaluate multiple
entities, dimensions, or elements (such as both coordinate axes, radii, or an
array of points), or that return multiple boolean outcomes (such as
std::pair<bool, bool>), use plural phrasing (e.g., are_axes_... or AreFinite).
Do not use singular phrasing (such as is_axis_...) when the query evaluates
multiple entities or returns per-entity results.

**Why.** Predicate and query names should grammatically match the subjects
evaluated and the results returned. Using a singular name like 'is_axis' for a
query that inspects multiple axes or returns per-axis results misleads callers
into assuming only a single entity is tested.

## Examples

Bad:

```cpp
// Singular name misleadingly implies only one axis is evaluated
std::pair<bool, bool> is_axis_nearly_integer_aligned(const LayerSpace<SkMatrix>& m,
                                                     LayerSpace<SkIPoint>* out=nullptr);

// Singular predicate inspecting multiple points
static bool IsFinite(const SkPoint pts[], int count);
```

Good:

```cpp
// Plural auxiliary verb and noun accurately reflect evaluating both axes
std::pair<bool, bool> are_axes_nearly_integer_aligned(const LayerSpace<SkMatrix>& m,
                                                      LayerSpace<SkIPoint>* out=nullptr);

// Plural phrasing matches multi-element array subject
static bool AreFinite(const SkPoint pts[], int count);
```

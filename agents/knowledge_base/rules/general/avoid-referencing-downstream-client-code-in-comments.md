---
name: skia/avoid-referencing-downstream-client-code-in-comments
description: When documenting algorithms or transformations in code comments, explain concepts in domain terms rather than referencing downstream client types or namespaces
rubric: readability
severity: nit
---

**Rule.** When documenting algorithms, mathematical transformations, or internal
logic in code comments, explain operations in self-contained domain terms rather
than referencing downstream client code, types, or namespaces (such as
Chromium's gfx::ColorSpace). Describe the underlying math, invariants, or
specifications directly so the explanation remains clear to all embedders.

**Why.** Skia is an embeddable graphics library consumed across diverse
platforms and projects (such as Chromium, Android, and Flutter). Referencing
downstream client types or namespaces in comments reduces clarity for developers
working on other embedders, breaks library encapsulation, and easily becomes
stale or misleading when client codebases evolve.

## Examples

Bad:

```cpp
// YDZDX follows multiplication order similar to YCbCr as Transfer * Inverse(Range) from
// gfx::ColorSpace.
void compute_matrix();
```

Good:

```cpp
// YDZDX applies range correction to YUV values similar to YCbCr.
void compute_matrix();
```

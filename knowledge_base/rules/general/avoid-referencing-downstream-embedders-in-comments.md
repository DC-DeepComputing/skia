---
name: skia/avoid-referencing-downstream-embedders-in-comments
description: When writing explanatory code comments, describe algorithms and domain logic self-containedly instead of referencing downstream embedder classes or namespaces like Chromium or Android
rubric: readability
severity: nit
---

**Rule.** Do not reference downstream embedding application code, classes, or
namespaces (such as Chromium's gfx::ColorSpace or Android/Flutter
implementations) in Skia comments. Describe mathematical operations, algorithm
steps, domain invariants, or specifications directly so that comments remain
self-contained for standalone maintenance.

**Why.** Skia is a standalone graphics library embedded across diverse client
applications, including Chromium, Android, and Flutter. Referencing downstream
embedder classes, namespaces, or implementation quirks couples comments to
external codebases that other embedders or standalone maintainers cannot inspect
and may not share.

## Examples

Bad:

```cpp
// GBR follows multiplication order similar to YCgCo as Inverse(Range) * Transfer from
// gfx::ColorSpace.
```

Good:

```cpp
// GBR applies range correction to RGB values similar to YCgCo.
```

---
name: skia/sksl/document-numeric-enum-branches
description: When branching on numeric scalar constants representing enum modes in SkSL shaders, label each branch with an inline comment and keep introductory comments generic
rubric: readability
severity: nit
---

**Rule.** When branching on numeric constants representing conceptual enum modes
in SkSL shaders, label each conditional branch with an inline comment naming the
mode (e.g., '// diffuse'). Keep introductory comments generalized rather than
enumerating a fixed subset of modes so they do not become stale as new variants
are added.

**Why.** In shader code, modes or enums from C++ are often passed as scalar
primitives (such as half or int), which obscures the conceptual variants being
handled. Labeling each branch clarifies the intent of magic numbers, and keeping
introductory comments generic prevents documentation drift when new cases are
introduced.

## Examples

Bad:

```cpp
// Diffuse and specular reflections scale the light's color differently
if (materialType == 0) {
    return color * diffuse;
} else if (materialType == 1) {
    return color * specular;
} else {
    return color * emboss;
}
```

Good:

```cpp
// Scale color by material type
if (materialType == 0) { // diffuse
    return color * diffuse;
} else if (materialType == 1) { // specular
    return color * specular;
} else { // emboss
    return color * emboss;
}
```

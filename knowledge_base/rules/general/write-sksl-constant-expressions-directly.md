---
name: skia/write-sksl-constant-expressions-directly
description: In SkSL shaders, write compile-time constant values directly as arithmetic expressions rather than manually precalculating them into decimal float literals
rubric: readability
severity: nit
---

**Rule.** In SkSL shaders, write compile-time constant values directly as
arithmetic expressions (such as 1.0 / 255.0 or 1.0 / (1 << 14)) rather than
manually precalculating them into decimal float literals with explanatory
comments.

**Why.** The SkSL compiler automatically performs constant folding at compile
time, and optimization passes inline or eliminate constant variables.
Precomputing arithmetic into decimal literals obscures mathematical intent and
risks precision or transcription errors without providing any runtime or shader
performance benefit.

## Examples

Bad:

```cpp
// Obscures intent and duplicates formula in comment
const half kInv255 = 0.003921569;  // 1.0 / 255.0
```

Good:

```cpp
// SkSL constant-folds the expression at compile time
const half kInv255 = 1.0 / 255.0;
```

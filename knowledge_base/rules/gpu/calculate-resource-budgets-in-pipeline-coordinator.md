---
name: skia/gpu/calculate-resource-budgets-in-pipeline-coordinator
description: When allocating stage resource budgets such as shader varyings in Graphite, calculate available capacity in the coordinator rather than hardcoding conservative limits in subroutines
rubric: design
severity: should_fix
---

**Rule.** Calculate pipeline resource budgets (such as available shader
varyings) in the coordinator layer (such as ShaderInfo) where all contributing
constraints (like RenderStep varyings and fixed reservations) are known. Pass
the computed remaining budget to lower-level subroutines rather than having
lower-level helpers hardcode conservative worst-case estimates.

**Why.** Lower-level helpers like PaintParamsKey lack context about the specific
pipeline stage or RenderStep being executed. Hardcoding conservative estimates
at the lower level unnecessarily restricts optimizations (such as lifting shader
expressions to vertex varyings) or risks exceeding hardware limits when pipeline
stages change.

## Examples

Bad:

```cpp
// In PaintParamsKey::getRootNodes(const Caps* caps, ...):
constexpr int kFixedVaryings = 2;
constexpr int kMaxStepVaryings = 6;
const int availableVaryings = caps->maxVaryings() - kFixedVaryings - kMaxStepVaryings;
fRootNodes = key.getRootNodes(caps, dict, alloc, availableVaryings);
```

Good:

```cpp
// In ShaderInfo::generateFragmentSkSL(const Caps* caps, ..., const RenderStep* step, ...):
constexpr int kFixedVaryings = 2;
const int availableVaryings = caps->maxVaryings() - kFixedVaryings - step->varyings().size();
fRootNodes = key.getRootNodes(caps, dict, alloc, availableVaryings);
```

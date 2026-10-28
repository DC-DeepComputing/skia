---
name: skia/gpu/document-precompile-placeholder-parameters
description: When passing fixed placeholder parameters to pipeline key block helpers during Graphite precompilation, document why combinations are not enumerated
rubric: readability
severity: should_fix
---

**Rule.** When passing fixed or placeholder parameter values (such as a constant
SkAlphaType or dummy texture info) to pipeline key block helpers (such as
AddBlock) during Graphite precompilation, document in an inline comment why the
parameter is hardcoded. Clarify that the value does not affect code snippet
selection or pipeline key structure, explaining why permutations (such as premul
vs. unpremul) do not need to be enumerated as precompile combinations.

**Why.** Precompilation enumerates parameter combinations to ensure all unique
shader snippet variants and pipeline layouts are prewarmed. Hardcoding
placeholder arguments into key data structures without explanation suggests that
valid combinations were overlooked or omitted. Documenting that snippet
selection is invariant to the parameter prevents confusion and avoids redundant
permutation loops that inflate compilation latency.

## Examples

Bad:

```cpp
// Bad: missing comment explaining why alpha type variants are not enumerated.
constexpr SkAlphaType kAlphaType = kPremul_SkAlphaType;
ColorSpaceTransformBlock::ColorSpaceTransformData csData(
        fSrc[srcCombination].get(), kAlphaType,
        fDst[dstCombination].get(), kAlphaType);
ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, csData);
```

Good:

```cpp
// Good: documents why alpha type combinations can safely be omitted.
// The alpha type is unused for determining which color space transform block to use.
constexpr SkAlphaType kAlphaType = kPremul_SkAlphaType;
ColorSpaceTransformBlock::ColorSpaceTransformData csData(
        fSrc[srcCombination].get(), kAlphaType,
        fDst[dstCombination].get(), kAlphaType);
ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, csData);
```

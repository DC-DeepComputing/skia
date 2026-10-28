---
name: skia/gpu/document-retained-precompile-combination-variants
description: When subsetting precompile combination counts to an integer literal based on flags in Graphite, add an inline comment identifying which named variant is retained
rubric: readability
severity: nit
---

**Rule.** In Graphite precompile classes, when calculating combination counts or
initializing combination fields with an integer literal because flags or options
exclude specific permutations, add an inline comment identifying the retained
variant (e.g., '// Just kHWTiled' or '// just kWithLocalMatrix').

**Why.** Combination counts in precompilation correspond to discrete named
variants (such as specific sampling or local matrix configurations). Using a raw
integer literal without an explanatory comment obscures which variant remains
active, hindering code reviews and making future precompilation audits
error-prone.

## Examples

Bad:

```cpp
// Bad: raw integer literal does not indicate which variant is retained.
: fNumExtraSamplingTilingCombos((flags & ImageShaderFlags::kExcludeCubic)
                                        ? 1
                                        : kExtraNumSamplingTilingCombos)
```

Good:

```cpp
// Good: documents which specific combination variant corresponds to the literal count.
: fNumExtraSamplingTilingCombos((flags & ImageShaderFlags::kExcludeCubic)
                                        ? 1  // Just kHWTiled
                                        : kExtraNumSamplingTilingCombos)
```

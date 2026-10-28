---
name: skia/vertically-align-nested-ternary-operators
description: When formatting chained or nested ternary operators across lines, break branches into an aligned ladder and vertically align matching '?' and ':' pairs
rubric: readability
severity: nit
---

**Rule.** When formatting nested or chained ternary operators, break each
condition and alternative branch onto separate lines as an aligned ladder rather
than compressing nested expressions onto a single line. Vertically align each
matching '?' and ':' operator pair according to its nesting level, without
arbitrarily indenting or outdenting ':' relative to its corresponding '?'.

**Why.** Inlining nested ternary branches onto a single line or arbitrarily
indenting matching '?' and ':' operator pairs obscures the conditional branching
hierarchy. Formatting chained branches as an aligned ladder with vertically
aligned operator pairs makes multi-branch conditional logic intuitive to read
and audit.

## Examples

Bad:

```cpp
// Inlining nested ternary branches onto a single line
int bindingIndex = texturesUseDistinctIdxRanges
        ? texIndex
        : separateSampler ? bufferOrGlobalIndex++ : bufferOrGlobalIndex;

// Arbitrarily outdented or misaligned '?' and ':' pairs
fColorInfos(!colorInfos.empty()
                    ? std::vector<SkColorInfo>(colorInfos.begin(), colorInfos.end())
            : raw ? kRawImageDefaultColorInfos
            : (flags & PrecompileImageShaderFlags::kExcludeAlpha)
                    ? kNonAlphaOnlyDefaultColorInfos
                    : kDefaultColorInfos)
```

Good:

```cpp
// Formatted as an aligned ladder with separated branches
int bindingIndex = texturesUseDistinctIdxRanges ? texIndex :
                                separateSampler ? bufferOrGlobalIndex++
                                                : bufferOrGlobalIndex;

// Matching '?' and ':' pairs vertically aligned at each nesting level
fColorInfos(!colorInfos.empty()
                    ? std::vector<SkColorInfo>(colorInfos.begin(), colorInfos.end())
                    : raw ? kRawImageDefaultColorInfos
                          : (flags & PrecompileImageShaderFlags::kExcludeAlpha)
                                    ? kNonAlphaOnlyDefaultColorInfos
                                    : kDefaultColorInfos)
```

---
name: skia/split-multiplexed-optional-helpers
description: When designing helper or factory functions, split disparate behaviors into dedicated, specialized functions instead of using optional parameters to multiplex distinct code paths
rubric: design
severity: should_fix
---

**Rule.** Prefer splitting helper or factory functions into explicit, dedicated
functions instead of using an optional parameter (such as std::optional<T> or a
nullable pointer) to multiplex disparate behaviors or special cases. Avoid
signatures where one execution branch ignores parameters required by another
branch or unnecessarily wraps the return value in std::optional for normal
callers whose path cannot fail.

**Why.** Using an optional parameter to multiplex unrelated branches forces
callers in special cases to pass unneeded or dummy arguments, complicates
function implementations with conditional branching, and forces infallible
callers to handle std::optional returns. Dedicated helpers give each function a
single responsibility, clean inputs, and exact return types.

## Examples

Bad:

```cpp
// Bad: Multiplexes standard conversion and Alpha8 special case via optional srcImageInfo.
std::optional<TargetInfo> makeTargetInfo(
        SkEncodedInfo dstInfo,
        const std::optional<SkImageInfo>& srcImageInfo,
        SkColorType dstCT,
        SkAlphaType dstAT) {
    if (srcImageInfo) {
        SkImageInfo dstRowInfo = SkImageInfo::Make(srcImageInfo->width(), 1, dstCT, dstAT);
        return TargetInfo{srcImageInfo->makeWH(srcImageInfo->width(), 1), dstRowInfo, dstInfo, dstRowInfo.minRowBytes()};
    } else {
        // dstCT and dstAT are unused here; handles only special Alpha8 case
        if (!safe.ok()) return std::nullopt;
        return TargetInfo{std::nullopt, std::nullopt, dstInfo, dstRowSize};
    }
}
```

Good:

```cpp
// Good: Split into dedicated helpers with explicit parameters and exact return types.
TargetInfo makeTargetInfo(SkEncodedInfo dstInfo, const SkImageInfo& srcImageInfo, SkColorType dstCT, SkAlphaType dstAT) {
    SkASSERT(dstCT != kAlpha_8_SkColorType);
    SkImageInfo dstRowInfo = SkImageInfo::Make(srcImageInfo.width(), 1, dstCT, dstAT);
    return TargetInfo{srcImageInfo.makeWH(srcImageInfo.width(), 1), dstRowInfo, std::move(dstInfo), dstRowInfo.minRowBytes()};
}

std::optional<TargetInfo> makeAlpha8TargetInfo(SkEncodedInfo dstInfo) {
    if (!safe.ok()) return std::nullopt;
    return TargetInfo{std::nullopt, std::nullopt, std::move(dstInfo), dstRowSize};
}
```

---
name: skia/gpu/encapsulate-capability-checks-in-decision-helpers
description: When evaluating hardware feature support in GPU pipeline keying, encapsulate capability and tile-mode checks inside decision helpers rather than inlining prerequisites at call sites
rubric: design
severity: should_fix
---

**Rule.** When determining whether a pipeline stage or draw operation can
execute in hardware, pass Caps or KeyContext into the decision helper and
encapsulate capability and mode checks directly within it. Do not calculate
prerequisite capability conditions in caller blocks to combine with helper
outcomes; extract shared predicates when multiple decision helpers evaluate
common constraints.

**Why.** Evaluating capability and mode constraints directly inside decision
helpers centralizes prerequisite logic, ensures all callers evaluate hardware
constraints consistently, and avoids duplicating ad-hoc capability queries
across shader generator call sites.

## Examples

Bad:

```cpp
bool shouldSubstituteDecal = !keyContext.caps()->clampToBorderSupport() &&
                             (imgData.fTileModes.first == SkTileMode::kDecal ||
                              imgData.fTileModes.second == SkTileMode::kDecal);
const bool doTilingInHw = !imgData.fSampling.useCubic && !shouldSubstituteDecal &&
                          can_do_yuv_tiling_in_hw(imgData);
```

Good:

```cpp
bool should_substitute_decal(const std::pair<SkTileMode, SkTileMode>& tileMode, const Caps* caps) {
    return !caps->clampToBorderSupport() && (tileMode.first == SkTileMode::kDecal ||
                                             tileMode.second == SkTileMode::kDecal);
}

static bool can_do_yuv_tiling_in_hw(const YUVImageShaderBlock::ImageData& imgData,
                                    const Caps* caps) {
    if (should_substitute_decal(imgData.fTileModes, caps)) {
        return false;
    }
    // ...
}

const Caps* caps = keyContext.caps();
const bool doTilingInHw = !imgData.fSampling.useCubic && can_do_yuv_tiling_in_hw(imgData, caps);
```

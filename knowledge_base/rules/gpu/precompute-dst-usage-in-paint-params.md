---
name: skia/gpu/precompute-dst-usage-in-paint-params
description: When constructing Graphite paint or draw parameter objects, precompute and store composite capability flags like DstUsage upfront rather than reconstructing them during key generation
rubric: design
severity: should_fix
---

**Rule.** When initializing Graphite draw and paint parameter descriptors (such
as PaintParams), precompute and store composite capability flags (such as
DstUsage) in the constructor using available Caps and texture format properties.
Do not store partial booleans or defer the re-evaluation of blend modes and
hardware blending capabilities into shader key generation (such as toKey).

**Why.** Caps, render target formats, and blend modes are already accessible
when constructing paint parameters. Precomputing composite capability flags
eliminates redundant intermediate fields, centralizes capability resolution
logic, and avoids re-inspecting blend mode properties during pipeline and shader
key generation.

## Examples

Bad:

```cpp
// Storing partial boolean in constructor and re-evaluating flags in toKey:
PaintParams::PaintParams(...)
    : ...
    , fDstReadReq(!CanUseHardwareBlending(caps, fTargetFormat, fFinalBlendMode, fRendererCoverage)) {}

std::optional<PaintParams::Result> PaintParams::toKey(...) const {
    DstUsage dstUsage;
    if (fDstReadReq) {
        dstUsage.fFlags |= DstUsage::Flags::kDstReadRequired;
    }
    if (fFinalBlendMode.has_value() && (int)fFinalBlendMode.value() > (int)SkBlendMode::kLastCoeffMode) {
        dstUsage.fFlags |= DstUsage::Flags::kAdvancedBlend;
    }
    return Result{paintID, dstUsage};
}
```

Good:

```cpp
// Precomputing DstUsage flags in constructor:
PaintParams::PaintParams(...)
    : ...
    , fDstUsage(get_dst_usage(caps, fTargetFormat, fFinalBlendMode, fRendererCoverage)) {}

std::optional<PaintParams::Result> PaintParams::toKey(...) const {
    return Result{paintID, fDstUsage | (dependsOnDst ? DstUsage::kDependsOnDst : DstUsage::kNone)};
}
```

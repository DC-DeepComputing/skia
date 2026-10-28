---
name: skia/locate-pipeline-effects-in-subsystem-directories
description: When implementing concrete pipeline effect subclasses like SkColorFilterBase or SkShaderBase, place them in src/effects/ or src/core/ rather than caller utility directories
rubric: design
severity: should_fix
---

**Rule.** Do not isolate concrete pipeline effect implementations (such as
subclasses of SkColorFilterBase, SkShaderBase, or SkMaskFilterBase) in utility
directories like src/utils/ simply because a utility is their initial or primary
caller. Locate effect subclasses alongside their type family in their dedicated
subsystem directories (such as src/effects/colorfilters/, src/effects/, or
src/core/).

**Why.** Pipeline effect implementations subclass core abstractions, participate
in raster and GPU pipeline lowering, and are registered in serialization tables.
Placing them in utility directories isolates them from sibling implementations,
breaks subsystem cohesion, and complicates bulk maintenance and backend
refactoring.

## Examples

Bad:

```cpp
// src/utils/SkGaussianColorFilter.h
class SkGaussianColorFilter final : public SkColorFilterBase {
public:
    static sk_sp<SkColorFilter> Make();
    bool appendStages(const SkStageRec&, bool) const override;
};
```

Good:

```cpp
// src/effects/colorfilters/SkGaussianColorFilter.h
class SkGaussianColorFilter final : public SkColorFilterBase {
public:
    static sk_sp<SkColorFilter> Make();
    bool appendStages(const SkStageRec&, bool) const override;
};
```

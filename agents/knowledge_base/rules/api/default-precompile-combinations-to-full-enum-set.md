---
name: skia/api/default-precompile-combinations-to-full-enum-set
description: When declaring public precompile APIs accepting enum combination spans, default parameters to complete enum sets instead of omitting variants based on internal pipeline optimizations
rubric: readability
severity: should_fix
---

**Rule.** In public precompile API declarations that accept enum spans to narrow
shader or pipeline permutations, default combination parameters to the full set
of enum values (e.g., via a constexpr array such as kAllTileModes) rather than a
partial subset based on internal pipeline optimizations. Handle redundant key
pruning and pipeline deduplication internally within the precompilation system
rather than leaking optimization shortcuts into public parameter defaults.

**Why.** Public precompile APIs allow callers to bound compilation combinations
to only the variants their application uses. Omitting valid enum values from
public defaults confuses callers into thinking omitted modes are unsupported,
and creates maintenance hazards and silent regressions if internal pipeline
generation changes in the future to specialize on those variants.

## Examples

Bad:

```cpp
// Leaking internal pipeline optimization into public API default parameter:
SK_API sk_sp<PrecompileShader> Image(
        ImageShaderFlags = ImageShaderFlags::kAll,
        SkSpan<const SkColorInfo> = {},
        SkSpan<const SkTileMode> = { SkTileMode::kClamp, SkTileMode::kRepeat });
```

Good:

```cpp
// Expose the complete enum set as default; deduplicate keys internally:
static constexpr SkTileMode kAllTileModes[] = {
    SkTileMode::kClamp,
    SkTileMode::kRepeat,
    SkTileMode::kMirror,
    SkTileMode::kDecal,
};

SK_API sk_sp<PrecompileShader> Image(
        ImageShaderFlags = ImageShaderFlags::kAll,
        SkSpan<const SkColorInfo> = {},
        SkSpan<const SkTileMode> = { kAllTileModes });
```

---
name: skia/use-consistent-nomenclature-for-paired-identifiers
description: When naming paired or variant identifiers along a scale, use consistent nomenclature across the set and align descriptors across related constants, enums, and keys
rubric: readability
severity: nit
---

**Rule.** When naming paired or complementary identifiers representing points
along a scale (such as size, bounds, or capacity), use consistent nomenclature
and abbreviation conventions across the set. Pair boundary extremes together
(such as Min with Max) and relative categories together (such as Sm with Lg, or
Small with Large) rather than mixing relative sizes with boundary extremes (such
as Sm with Max). Additionally, align variant descriptors across related
constants, enums, and keys that govern or represent the same tiers (e.g., match
Small/Large constants with Sm/Lg keys, rather than introducing divergent terms
like Medium).

**Why.** Mixing conceptual dimensions (such as pairing relative size 'Sm' with
boundary extreme 'Max') or using divergent tier descriptors between related
constants and keys (such as 'Medium' for a 'Sm' key) produces confusing APIs and
obscures which constant configures which variant. Aligning nomenclature keeps
variant relationships predictable and symmetrical across the codebase.

## Examples

Bad:

```cpp
// Bad: mixes relative size category 'Sm' with boundary extreme 'Max'.
enum class FilterMode {
    kMatrixConvTexSm,
    kMatrixConvTexMax,
};

// Bad: mismatched tier descriptors between constant ('Medium') and key ('Sm').
static constexpr int kMaxKernelSize = 256;
static constexpr int kMediumKernelSize = 64;
M(MatrixConvTexSm)
M(MatrixConvTexMax)
```

Good:

```cpp
// Good: pairs relative size categories consistently ('Sm' with 'Lg').
enum class FilterMode {
    kMatrixConvTexSm,
    kMatrixConvTexLg,
};

// Good: aligned symmetrical descriptors across constants and keys.
static constexpr int kLargeKernelSize = 256;
static constexpr int kSmallKernelSize = 64;
M(MatrixConvTexSm)
M(MatrixConvTexLg)
```

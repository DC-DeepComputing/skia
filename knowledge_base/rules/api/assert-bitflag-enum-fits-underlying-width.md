---
name: skia/api/assert-bitflag-enum-fits-underlying-width
description: When packing enums into bitfields or defining fixed-width bitflag enums, verify with static_assert that enumerators or counts fit the bit width
rubric: readability
severity: should_fix
---

**Rule.** When defining fixed-width bitflag enums or packing enum values into
struct bitfields and bit-packed integers, add a static_assert verifying that the
maximum enumerator or enum count (e.g., kCount or kSize) fits within the
allocated bit capacity (such as (1 << kBits) or the underlying integer type).
Place the assertion alongside the enum definition or bitfield declaration to
catch overflow at compile time.

**Why.** Adding new enumerators or expanding flag enums can silently exceed the
bitfield width or underlying integer capacity, causing silent truncation or bit
bleeding into adjacent fields. Verifying bounds at compile time with
static_assert prevents overflow bugs across all build configurations.

## Examples

Bad:

```cpp
// Variant 1: Fixed-width bitflag enum
enum InternalDrawTypeFlags : uint16_t {
    kCoverageMask  = DrawTypeFlags::kLast << 1,
    kAnalyticRRect = DrawTypeFlags::kLast << 2,
};

// Variant 2: Enum packed into a bitfield
enum class GlyphAction { kUnset, kAccept, kReject, kDrop, kSize };
struct {
    uint32_t fPathAction : 2;  // GlyphAction
};
```

Good:

```cpp
// Variant 1: Fixed-width bitflag enum
enum InternalDrawTypeFlags : uint16_t {
    kCoverageMask  = DrawTypeFlags::kLast << 1,
    kAnalyticRRect = DrawTypeFlags::kLast << 2,
    kLastInternal = kAnalyticRRect,
};
static_assert(kLastInternal <= (1 << 15), "DrawTypeFlags do not fit in 16 bits");

// Variant 2: Enum packed into a bitfield
enum class GlyphAction { kUnset, kAccept, kReject, kDrop, kSize };
static_assert(SkTo<int>(GlyphAction::kSize) <= (1 << 2));
struct {
    uint32_t fPathAction : 2;  // GlyphAction
};
```

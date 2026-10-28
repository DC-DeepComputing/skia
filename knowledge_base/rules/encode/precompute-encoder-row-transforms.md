---
name: skia/encode/precompute-encoder-row-transforms
description: When implementing image encoders with per-row processing, resolve row transformation configurations upfront during factory initialization instead of re-evaluating pixmap metadata inside onEncodeRow
rubric: design
severity: should_fix
---

**Rule.** When implementing image encoders and streaming row processors, resolve
and record row-transformation configurations (such as color type conversions,
channel stripping, or endianness swaps) upfront during factory initialization
(e.g. in Make()) into dedicated enum modes or function pointers. Do not
re-inspect pixmap or destination metadata (such as colorType() or isOpaque())
repeatedly within the inner onEncodeRow() or onEncodeRows() hot path.

**Why.** Evaluating pixmap properties on every row introduces redundant
branching, complex nested conditions, and unnecessary overhead into the
performance-sensitive row encoding loop. Precomputing the transformation during
initialization simplifies onEncodeRow to a direct switch or callback dispatch on
a known mode.

## Examples

Bad:

```cpp
bool SkPngRustEncoderImpl::onEncodeRow(SkSpan<const uint8_t> row) {
    // Anti-pattern: re-evaluating image info and re-branching on every row
    if (this->targetInfo().fDstRowInfo.has_value()) {
        switch (this->targetInfo().fDstRowInfo->colorType()) {
            case kR16G16B16A16_unorm_SkColorType:
                if (this->targetInfo().fDstRowInfo->isOpaque()) {
                    // swap endianness and strip alpha ...
                }
                break;
            default:
                break;
        }
    }
    return true;
}
```

Good:

```cpp
// In Make(): resolve configuration upfront into an enum
ExtraRowTransform transform = kNone_ExtraRowTransform;
if (maybeDstRowInfo->isOpaque()) {
    transform = (maybeDstRowInfo->colorType() == kR16G16B16A16_unorm_SkColorType)
        ? kRgba16leToRgb16be_ExtraRowTransform
        : kRgba8ToRgb8_ExtraRowTransform;
} else if (maybeDstRowInfo->colorType() == kR16G16B16A16_unorm_SkColorType) {
    transform = kRgba16leToRgba16be_ExtraRowTransform;
}

// In onEncodeRow(): single dispatch on precomputed transform mode
switch (this->fExtraRowTransform) {
    case kRgba8ToRgb8_ExtraRowTransform: ...
    case kRgba16leToRgb16be_ExtraRowTransform: ...
    case kNone_ExtraRowTransform: ...
}
```

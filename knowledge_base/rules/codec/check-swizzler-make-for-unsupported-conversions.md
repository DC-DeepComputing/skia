---
name: skia/codec/check-swizzler-make-for-unsupported-conversions
description: When initializing an SkSwizzler in image decoders via SkSwizzler::Make or MakeSimple, check for nullptr and return kInvalidConversion instead of asserting with SkASSERT
rubric: correctness
severity: should_fix
---

**Rule.** When creating an SkSwizzler via SkSwizzler::Make or
SkSwizzler::MakeSimple in image decoders, do not assume creation always succeeds
using SkASSERT. Check if the returned swizzler pointer is null and return an
error result such as SkCodec::kInvalidConversion or kUnimplemented to handle
unsupported format conversions gracefully.

**Why.** While SkCodec::conversionSupported permits certain destination color
types, SkSwizzler only supports a subset of format conversions and returns
nullptr for unsupported configurations (such as kBGRA_10101010_XR_SkColorType).
Relying on SkASSERT results in assertion failures in debug builds and null
pointer dereferences in release builds when callers request these formats.

## Examples

Bad:

```cpp
fSwizzler = SkSwizzler::Make(this->getEncodedInfo(), nullptr, swizzlerInfo, swizzlerOptions);
SkASSERT(fSwizzler);
return kSuccess;
```

Good:

```cpp
fSwizzler = SkSwizzler::Make(this->getEncodedInfo(), nullptr, swizzlerInfo, swizzlerOptions);
if (!fSwizzler) {
    return kInvalidConversion;
}
return kSuccess;
```

---
name: skia/gpu/encapsulate-helper-context-configuration
description: When validating buffer layout constraints and configuring internal execution context on helper structs, encapsulate calculations and field updates within helper member methods
rubric: design
severity: should_fix
---

**Rule.** Encapsulate state validation and context configuration (such as
alignment checking and stride calculation) within member methods on helper
structs rather than having callers directly inspect internal properties and
mutate context fields. The helper method should verify preconditions and update
its internal execution contexts, returning success status or updated parameters
to caller control flow.

**Why.** Directly inspecting internal fields (such as bytes-per-pixel) of an
embedded helper struct to validate alignment and assign context properties
breaks encapsulation and couples callers to layout details. Encapsulating
configuration logic within a member method on the helper struct keeps internal
members private or const and simplifies caller control flow.

## Examples

Bad:

```cpp
// Caller directly checks helper fields and mutates internal context:
if (fRP && !(fPreOps || fPostOps) &&
    srcRowBytes % fRP->fSrcBpp == 0 && dstRowBytes % fRP->fDstBpp == 0) {
    rpInvokeCount = 1;
    fRP->fSrcCtx.stride = SkTo<int>(srcRowBytes / fRP->fSrcBpp);
    fRP->fDstCtx.stride = SkTo<int>(dstRowBytes / fRP->fDstBpp);
}
```

Good:

```cpp
// Member method encapsulates validation and context updates:
bool TextureFormatXferFn::RPOps::setStrides(size_t srcRowBytes, size_t dstRowBytes, uint8_t otherOps) {
    if (otherOps == 0 && srcRowBytes % fSrcBpp == 0 && dstRowBytes % fDstBpp == 0) {
        fSrcCtx.stride = SkTo<int>(srcRowBytes / fSrcBpp);
        fDstCtx.stride = SkTo<int>(dstRowBytes / fDstBpp);
        return true;
    }
    fSrcCtx.stride = 0;
    fDstCtx.stride = 0;
    return false;
}

// Caller delegates to helper member method:
if (fRP && fRP->setStrides(srcRowBytes, dstRowBytes, fPreOps | fPostOps)) {
    rpInvokeCount = 1;
}
```

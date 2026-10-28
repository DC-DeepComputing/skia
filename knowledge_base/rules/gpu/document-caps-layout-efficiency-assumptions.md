---
name: skia/gpu/document-caps-layout-efficiency-assumptions
description: When hardcoding resource layout or tiling efficiency assumptions in GPU backend Caps, document the physical memory invariant explaining why driver queries are bypassed
rubric: readability
severity: nit
---

**Rule.** In GPU backend capability checks (such as Caps or format query
helpers), when branching on resource tiling modes or memory layouts where
certain layouts are unconditionally assumed efficient or supported while others
query driver performance metrics, document the underlying physical memory layout
invariant in an accompanying comment. Explicitly explain why the bypassed layout
is inherently efficient (e.g., explaining that linear tiling reduces host image
copies to sequential memcpy operations without decompression or re-tiling
overhead) to clarify why driver performance queries are unnecessary.

**Why.** Querying driver performance properties for optimal layouts while
unconditionally returning true or hardcoding efficiency for linear layouts
appears contradictory or incomplete without context. Documenting the hardware or
memory layout invariants (such as linear data mapping directly to sequential
memory copies) prevents reviewers from misinterpreting hardcoded assumptions as
bugs or attempting to add redundant driver queries.

## Examples

Bad:

```cpp
// In GPU Caps capability query:
isEfficientWithHostImageCopy = tiling == Tiling::kLinear ||
                               this->isEfficientWithHostCopy(interface, physDev, vkFormat);

switch (imageTiling) {
    case VK_IMAGE_TILING_OPTIMAL:
        return fIsEfficientWithHostImageCopy;
    case VK_IMAGE_TILING_LINEAR:
        return true;
}
```

Good:

```cpp
// In GPU Caps capability query:
// Assume linear tiling is always efficient for copying directly via sequential memcpys.
isEfficientWithHostImageCopy = tiling == Tiling::kLinear ||
                               this->isEfficientWithHostCopy(interface, physDev, vkFormat);

switch (imageTiling) {
    case VK_IMAGE_TILING_OPTIMAL:
        return fIsEfficientWithHostImageCopy;
    case VK_IMAGE_TILING_LINEAR:
        // Host-image-copy is always efficient with linear tiling as it avoids re-tiling penalties.
        return true;
}
```

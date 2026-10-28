---
name: skia/value-initialize-partially-loaded-simd-vectors
description: When declaring SIMD vectors or stack transfer buffers that may be partially populated, value-initialize them with {} to prevent uninitialized memory reads under MSan
rubric: safety
severity: should_fix
---

**Rule.** When declaring SIMD vector types (such as `skvx::Vec<N, T>`) or stack
transfer buffers that might be only partially populated before SIMD
processing—such as handling loop tails or formats with fewer channels than
vector capacity—value-initialize them with `{}` at declaration (`PxVec
pixel{};`). Do not leave them default-initialized, as unwritten lanes or bytes
retain indeterminate values that trigger MemorySanitizer (MSan) errors or
undefined behavior during subsequent vector operations.

**Why.** Vector types like `skvx::Vec` use defaulted default constructors
(`Vec() = default`), leaving stack instances uninitialized when declared without
braces. When subsequent operations only partially write the vector (e.g. via
`memcpy` or partial loads), SIMD arithmetic, shuffles, or stores read
uninitialized lanes, causing MSan failures or non-deterministic calculation
results.

## Examples

Bad:

```cpp
// PxVec (skvx::Vec) left uninitialized on the stack
PxVec pixel;
while (width >= N) {
    memcpy(&pixel, src, srcBppN);
    pixel = applyPixel(pixel, opaqueAlpha);
    memcpy(dst, &pixel, dstBppN);
    ...
}
if (width > 0) {
    // Partial copy leaves trailing lanes uninitialized; applyPixel reads garbage
    memcpy(&pixel, src, width * srcBpp);
    pixel = applyPixel(pixel, opaqueAlpha);
    memcpy(dst, &pixel, width * dstBpp);
}
```

Good:

```cpp
// Value-initialize with {} so all lanes are zeroed
PxVec pixel{};
while (width >= N) {
    memcpy(&pixel, src, srcBppN);
    pixel = applyPixel(pixel, opaqueAlpha);
    memcpy(dst, &pixel, dstBppN);
    ...
}
if (width > 0) {
    // Unwritten lanes remain safely zeroed across partial copy and SIMD ops
    memcpy(&pixel, src, width * srcBpp);
    pixel = applyPixel(pixel, opaqueAlpha);
    memcpy(dst, &pixel, width * dstBpp);
}
```

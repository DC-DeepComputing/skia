---
name: skia/core/preserve-src-dst-order-in-blend-procs
description: When invoking non-commutative pixel or alpha blending function pointers taking (src, dst), pass source before destination to avoid inverting the blend operation
rubric: correctness
severity: blocking
---

**Rule.** When invoking pixel or alpha blending function pointers and callbacks
defined with `(src, dst)` parameter order (such as `AlphaProc`), pass the source
value as the first argument and destination as the second argument. Do not swap
arguments when calling non-commutative blend procs (such as `kSrcOver` or
`kSrc`).

**Why.** Because Porter-Duff blend modes like `kSrcOver` and `kSrc` are
non-commutative, passing destination before source computes
destination-over-source or destination-copy instead of source-over-destination,
corrupting the blended pixel color or alpha.

## Examples

Bad:

```cpp
// Inverted arguments: passing dst (*device) before src (fSrc) to AlphaProc(src, dst)
*device = fOneProc(*device, fSrc);
```

Good:

```cpp
// Correct argument order: src (fSrc) passed before dst (*device)
*device = fOneProc(fSrc, *device);
```

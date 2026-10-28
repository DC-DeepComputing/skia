---
name: skia/opts/opts-name-kernel-parameters-by-dataflow
description: When implementing pixel-processing or blending kernels and lambdas, name input parameters src and outputs dst based on dataflow rather than destination buffer origin
rubric: readability
severity: nit
---

**Rule.** In pixel-processing and blending kernels or helper lambdas, name input
parameters src and return values dst (or output) to reflect natural dataflow,
rather than naming the parameter dst because it was loaded from a destination
buffer. When a kernel performs an in-place blend on pixels loaded from a
destination buffer, add an inline comment explaining caller-side context instead
of reversing parameter naming.

**Why.** From the perspective of a kernel or helper lambda, the argument is the
incoming value (src) and the return value is the transformed output (dst).
Naming the input parameter dst reverses natural dataflow conventions and creates
confusion when reading the blend formula; keeping parameter names aligned with
dataflow and clarifying caller context in comments maintains consistency across
blending routines.

## Examples

Bad:

```cpp
// Kernel parameter named 'dst' because caller loads from destination buffer
auto kernel = [color](U32 dst) {
    U8 d = sk_bit_cast<U8>(dst),
       a = U8(invA);
    U16 c = skvx::cast<uint16_t>(sk_bit_cast<U8>(U32(color))),
        r = (mull(d, a) >> 8) + c;
    return sk_bit_cast<U32>(skvx::cast<uint8_t>(r));
};
```

Good:

```cpp
// Note when the kernel is used below, the "src" is the existing pixel color.
auto kernel = [color](U32 src) {
    U8 s = sk_bit_cast<U8>(src),
       a = U8(invA);
    U16 c = skvx::cast<uint16_t>(sk_bit_cast<U8>(U32(color))),
        r = (mull(s, a) >> 8) + c;
    return sk_bit_cast<U32>(skvx::cast<uint8_t>(r));
};
```

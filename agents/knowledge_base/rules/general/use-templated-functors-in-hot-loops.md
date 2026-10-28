---
name: skia/use-templated-functors-in-hot-loops
description: When passing callables applied inside performance-critical inner loops or pixel row processors, use templated functors instead of function pointers to enable compiler inlining
rubric: safety
severity: nit
---

**Rule.** When implementing or parameterizing operations executed repeatedly
inside hot inner loops (such as pixel row, color conversion, or SIMD processing
loops), accept the callable as a templated parameter (e.g., template <typename
PixelFn>) rather than as a raw function pointer (e.g., Px(*)(Px)) or
std::function. Pass and invoke the callable directly so the compiler can
monomorphize the loop, inline the operation, and vectorize iterations.

**Why.** Invoking operations through a function pointer inside an inner loop
introduces indirect function call overhead on every iteration and prevents the
compiler from inlining the callable, unrolling the loop, or performing SIMD
vectorization. Using a template parameter allows the compiler to monomorphize
the loop for the concrete callable and inline it directly into the inner loop
body.

## Examples

Bad:

```cpp
template <typename PxVec>
using ApplyPixelFn = PxVec(*)(PxVec);

template <typename PxVec>
XferRowFn create_xfer_row_fn(int n, int srcBpp, int dstBpp, ApplyPixelFn<PxVec> fn) {
    return [n, srcBpp, dstBpp, fn](const char* src, char* dst, int width) {
        while (width >= n) {
            memcpy(&pixel, src, srcBppN);
            pixel = fn(pixel);
            memcpy(dst, &pixel, dstBppN);
        }
    };
}
```

Good:

```cpp
template <typename PxVec, typename PixelFn>
XferRowFn create_xfer_row_fn(int n, int srcBpp, int dstBpp, PixelFn applyPixel) {
    return [n, srcBpp, dstBpp, applyPixel](const char* src, char* dst, int width) {
        while (width >= n) {
            memcpy(&pixel, src, srcBppN);
            pixel = applyPixel(pixel);
            memcpy(dst, &pixel, dstBppN);
        }
    };
}
```

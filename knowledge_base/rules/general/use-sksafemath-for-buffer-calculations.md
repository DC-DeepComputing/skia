---
name: skia/use-sksafemath-for-buffer-calculations
description: When computing or validating buffer sizes, strides, offsets, or dimension limits, use SkSafeMath or static SkSafeMath::Mul, verifying safe.ok() before allocation
rubric: safety
severity: blocking
---

**Rule.** When computing or validating buffer sizes, row bytes, strides,
dimensions, or bounds limits (such as width * height), use SkSafeMath (e.g.,
safe.mul(), safe.add(), SkSafeAccumulator, or static SkSafeMath::Mul/Add)
instead of raw arithmetic or ad-hoc 64-bit checks. Verify safe.ok() before
allocating memory, indexing, or slicing (e.g., SkSpan::subspan()), and place
!safe.ok() first in compound validation conditionals. For inline bounds checks
against maximum limits, use static SkSafeMath::Mul to saturate to SIZE_MAX on
overflow. Validate signed inputs with safe.castTo<size_t>(), and use
safe.addInt()/mulInt() or safe.castTo<T>() for signed/narrowing results.

**Why.** Raw integer arithmetic on dimensions, strides, or counts can overflow
and wrap around, bypassing upper-bound limits and causing undersized allocations
or buffer overflows. SkSafeMath detects overflow via safe.ok(), while static
SkSafeMath::Mul saturates to SIZE_MAX on overflow so comparisons against maximum
bounds fail safely. Consuming unvalidated results or evaluating comparisons
before checking !safe.ok() risks memory corruption or crashes.

## Examples

Bad:

```cpp
// Bad: raw dimension multiplication in bounds check can overflow and bypass limit
if (kernelSize.width() * kernelSize.height() > kMaxKernelSize) {
    return nullptr;
}

// Bad: evaluating comparisons before checking !safe.ok() in compound conditionals
if (mpEntriesData->size() != expectedSize || !expectedSizeSafe.ok()) {
    return false;
}

// Bad: slicing span before verifying safe.ok() crashes on fuzzed/overflowed offsets
decodingState->fDst = SkSpan(static_cast<uint8_t*>(pixels), imageSize)
                              .subspan(xByteOffset)
                              .subspan(yByteOffset);
if (!safe.ok()) {
    return kErrorInInput;
}

// Bad: ad-hoc 64-bit casting or raw arithmetic for buffer allocation
int64_t width64 = (int64_t)4 * devBlurRadius + devLeft + devRight + 1;
if (width64 > SK_MaxS32) {
    return false;
}
void* storage = ::operator new(npts * sizeof(SkPoint));

// Bad: passing signed input to size_t or unchecked narrowing cast
const size_t srcRowBytes = width * bytesPerPixel;
int width = (int)safe.add((size_t)devLeft, (size_t)devRight);
```

Good:

```cpp
// Good: static SkSafeMath::Mul returns SIZE_MAX on overflow so bounds check fails
if (SkSafeMath::Mul(kernelSize.width(), kernelSize.height()) > kMaxKernelSize) {
    return nullptr;
}

// Good: check !safe.ok() first to short-circuit before comparing computed values
if (!expectedSizeSafe.ok() || mpEntriesData->size() != expectedSize) {
    return false;
}

// Good: verify safe.ok() before passing computed offsets to bounded subspan()
if (!safe.ok()) {
    return kErrorInInput;
}
decodingState->fDst = SkSpan(static_cast<uint8_t*>(pixels), imageSize)
                              .subspan(xByteOffset)
                              .subspan(yByteOffset);

// Good: SkSafeMath/SkSafeAccumulator with safe.ok() before allocation
SkSafeMath safe;
int width_safe = safe.addInt(devLeft, devRight);
width_safe = safe.addInt(width_safe, safe.mulInt(4, devBlurRadius));
if (!safe.ok()) {
    return false;
}

// Good: safe conversions and typed operations
const size_t srcRowBytes = safe.mul(safe.castTo<size_t>(width), bytesPerPixel);
```

---
name: skia/prefer-static-assert-for-compile-time-invariants
description: When validating compile-time constants, type sizes, traits, or structural invariants, use static_assert rather than runtime SkASSERT to catch violations at build time
rubric: correctness
severity: should_fix
---

**Rule.** When validating conditions known at compile time—such as type sizes
(sizeof), alignments (alignof), type traits, platform constants, or template
parameters—use static_assert rather than runtime assertions like SkASSERT or
SkASSERT_RELEASE. Place static_assert at class, template, or function scope so
violations fail immediately at build time across all configurations. When
algorithms, cache keys, or buffer alignments rely on compile-time configuration
constants satisfying mathematical properties (such as power-of-two sizing via
SkIsPow2, divisibility, or relative limits), enforce them with static_assert.
Additionally, compute aggregate element counts explicitly as sizeof(T) /
sizeof(ElementType) and assert divisibility or size constraints with
static_assert.

**Why.** Runtime assertions like SkASSERT only execute if the containing code
path is exercised during a debug run, silently permitting invalid
configurations, constant modifications, or template instantiations to build and
slip into release binaries. Validating compile-time invariants with
static_assert catches misconfigurations immediately at build time with zero
runtime cost, while expressing element counts as sizeof(T) / sizeof(ElementType)
avoids assuming 1-byte channels.

## Examples

Bad:

```cpp
// Bad: Assumes 1-byte elements implicitly and uses runtime SkASSERT for compile-time invariants
template <typename T>
class GaussianPass final : public Pass {
    static constexpr int N = sizeof(T);
    static PassMaker* MakeMaker(float sigma, SkArenaAlloc* alloc) {
        SkASSERT(CHAR_BIT == 8);
        SkASSERT(sizeof(T) <= 16);
        return alloc->make<GaussianPass<T>>(sigma);
    }
};

// Bad: Relying on runtime SkASSERT or unasserted assumptions for compile-time constant properties
SkASSERT(texWidth <= kMaxKernelDimension && texHeight <= kMaxKernelDimension);
const SkISize key = {SkNextPow2(texWidth), SkNextPow2(texHeight)};
```

Good:

```cpp
// Good: static_assert validates compile-time type invariants across all builds
template <typename T>
class GaussianPass final : public Pass {
    static constexpr int N = sizeof(T) / sizeof(uint8_t);
    static_assert(N * sizeof(uint8_t) == sizeof(T), "N must be size of T in bytes");
    static_assert(sizeof(T) <= 16, "Type T exceeds maximum supported filter size");

    static PassMaker* MakeMaker(float sigma, SkArenaAlloc* alloc) {
        return alloc->make<GaussianPass<T>>(sigma);
    }
};

// Good: Enforce mathematical properties of compile-time constants at build time
static_assert(SkIsPow2(kMaxKernelDimension), "kMaxKernelDimension must be power of two");
SkASSERT(texWidth <= kMaxKernelDimension && texHeight <= kMaxKernelDimension);
const SkISize key = {SkNextPow2(texWidth), SkNextPow2(texHeight)};
```

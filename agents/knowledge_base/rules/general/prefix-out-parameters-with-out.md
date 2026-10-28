---
name: skia/prefix-out-parameters-with-out
description: When declaring functions with mutable pointer out-parameters, prefix parameter names with out (e.g., outAT) to distinguish them from optional input pointers
rubric: readability
severity: nit
---

**Rule.** Prefix out-parameter and in-out pointer arguments with `out` (e.g.,
`outAT`, `outColor`, or `outFoo`) in function declarations and definitions. Do
not use bare or unadorned names for output pointers to ensure they are clearly
distinguished from optional input pointer parameters.

**Why.** In Skia C++ conventions, raw pointers can represent either optional
input parameters (const T*) or mutable output parameters (T*). Prefixing output
parameters with `out` makes argument directionality immediately obvious in
declarations and at call sites without requiring readers to inspect the function
implementation.

## Examples

Bad:

```cpp
// Bad: raw pointer out-parameter uses a bare name, easily confused with optional input pointers
sk_sp<SkColorSpace> workingFormat(const sk_sp<SkColorSpace>& dstCS,
                                  SkAlphaType* at) const;

bool decodeHeader(SkStream* stream, SkISize* size);
```

Good:

```cpp
// Good: out-parameter is explicitly prefixed with 'out'
sk_sp<SkColorSpace> workingFormat(const sk_sp<SkColorSpace>& dstCS,
                                  SkAlphaType* outAT) const;

bool decodeHeader(SkStream* stream, SkISize* outSize);
```

---
name: skia/avoid-plain-unsigned-for-counters-and-indices
description: When declaring variables, loop counters, or sizing containers and spans, avoid plain unsigned; use int for general counts and size_t for spans and indexing
rubric: readability
severity: should_fix
---

**Rule.** Prefer standard `int` over fixed-width integer types (such as
`uint8_t` or `int32_t`) and plain `unsigned` (or `unsigned int`) for general
integers, member fields, and resource counts unless a specific bit-width
guarantee or memory packing is required. Assert that counts are non-negative
(using `SkASSERT`) instead of using unsigned types. For loop induction
variables, collection sizes, span lengths, and indexing, avoid plain `unsigned`
(or `(unsigned)` casts); use `size_t` (or `SkToSizeT`) when sizing or indexing
arrays, standard containers, or spans, and `int` for general iterations and
counts.

**Why.** Skia style follows the Google C++ style guide for integer types,
preferring standard `int` to eliminate signed/unsigned comparison warnings and
avoid cast boilerplate. Plain `unsigned` is non-portable, risks truncating
64-bit values, and differs in width from pointer-sized `size_t`, while arbitrary
fixed-width types like `uint8_t` add unnecessary cognitive overhead unless
memory layout or bit-width guarantees strictly require them. Standard containers
and `SkSpan` operate on `size_t`; using `size_t` avoids truncation and type
mismatches.

## Examples

Bad:

```cpp
// Bad: plain unsigned for loop counter and array indexing
for (unsigned index = 1; index <= VerbPtCount(verb); ++index) {
    if (pts[0].fY != pts[index].fY) { ... }
}

// Bad: plain unsigned cast when constructing span or container
this->reset({that.data(), (unsigned)that.size()});

// Bad: fixed-width unsigned type for general resource count
class ResourceTracker {
    SkString fResourceTypeSubstr;
    uint8_t fNumResourcesWithTypeSubstr = 0;
};
```

Good:

```cpp
// Good: use size_t when indexing arrays/spans, or int for general counts
for (size_t index = 1; index <= VerbPtCount(verb); ++index) {
    if (pts[0].fY != pts[index].fY) { ... }
}

// Good: use size_t when sizing containers or spans
this->reset({that.data(), (size_t)that.size()});

// Good: use int for general counts and assert non-negativity if needed
class ResourceTracker {
    SkString fResourceTypeSubstr;
    int fNumResourcesWithTypeSubstr = 0;
};
```

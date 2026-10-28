---
name: skia/pass-arrays-directly-to-skspan
description: When passing or constructing an SkSpan, pass arrays directly, pass scalars as {&val, 1}, and default-construct empty spans ({}) instead of {nullptr, 0}
rubric: readability
severity: should_fix
---

**Rule.** When passing a fixed-size C-style array (such as T[N]), std::array, or
standard contiguous container to a function taking SkSpan<T>, pass the container
directly rather than wrapping it in an explicit {ptr, count} brace pair or
redundant SkSpan constructor. When passing an individual scalar variable to an
SkSpan parameter, pass an inline singleton span {&val, 1} instead of declaring a
1-element C array (T val[1]). When creating, returning, or passing an empty
SkSpan, prefer default construction (SkSpan<T>() or {}) over explicitly passing
{nullptr, 0}.

**Why.** SkSpan provides implicit constructors for fixed-size arrays and
standard containers, making manual {ptr, count} wrappers redundant. For single
scalar variables, passing {&val, 1} operates directly without declaring
unnecessary 1-element arrays. For empty spans, SkSpan's constexpr default
constructor already initializes the pointer to nullptr and size to 0, making
explicit {nullptr, 0} redundant and less idiomatic.

## Examples

Bad:

```cpp
// Redundant brace pair for fixed-size array
SkVector vec[2];
localMatrix.mapVectors({vec, 2});

// 1-element array declared solely for implicit span conversion
SkPoint3 dst[1];
this->mapHomogeneousPoints(dst, {&src, 1});

// Explicit {nullptr, 0} for empty span
return fVars ? SkSpan<const SkString>(fVars->fData) : SkSpan<const SkString>{nullptr, 0};
```

Good:

```cpp
// Pass fixed-size array directly
SkVector vec[2];
localMatrix.mapVectors(vec);

// Pass scalar as inline singleton span without 1-element array
SkPoint3 dst;
this->mapHomogeneousPoints({&dst, 1}, {&src, 1});

// Default-construct empty span
return fVars ? SkSpan<const SkString>(fVars->fData) : SkSpan<const SkString>();
```

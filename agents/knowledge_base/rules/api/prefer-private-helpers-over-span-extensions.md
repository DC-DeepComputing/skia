---
name: skia/api/prefer-private-helpers-over-span-extensions
description: When implementing utility operations for transitional standard types like SkSpan, place internal helpers in private utility headers instead of adding member methods to public classes
rubric: design
severity: should_fix
---

**Rule.** Do not add custom convenience member methods (such as equals()) to
public transitional vocabulary types like SkSpan. Implement internal span
operations, comparisons, or copy routines as static non-member helpers in
private headers (such as SkSpanPriv::EQ in SkSpanPriv.h) or standard algorithms
instead.

**Why.** SkSpan is designed to align with and transition directly to C++20
std::span, and may be directly aliased (template <typename T> using SkSpan =
std::span<T>). Adding bespoke member functions to public SkSpan causes interface
divergence, breaks compilation when aliased to std::span, and creates migration
friction across the codebase.

## Examples

Bad:

```cpp
// include/core/SkSpan.h
template <typename T>
class SkSpan {
public:
    // Anti-pattern: adding custom member methods to a transitional standard type
    bool equals(SkSpan<T> other) const;
};
```

Good:

```cpp
// src/core/SkSpanPriv.h
class SkSpanPriv {
public:
    template <typename T>
    static bool EQ(SkSpan<T> a, SkSpan<T> b) {
        return (a.size() == b.size()) &&
               (a.empty() || a.data() == b.data() || std::equal(a.begin(), a.end(), b.begin()));
    }
};
```

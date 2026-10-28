---
name: skia/avoid-std-tie-in-header-comparisons
description: When implementing struct comparison operators in headers, write explicit member comparisons instead of including <tuple> for std::tie to minimize dependencies and support constexpr
rubric: readability
severity: nit
---

**Rule.** Avoid including `<tuple>` in header files solely to implement
comparison operators (such as `operator==` or `operator<`) via `std::tie`.
Prefer writing explicit direct member comparisons, which avoids pulling in heavy
template dependencies and allows marking the comparison operators `constexpr`
for compile-time validation.

**Why.** Skia guidelines emphasize minimizing header includes to reduce
compilation cascades and translation unit bloat. In addition to avoiding the
heavy `<tuple>` header, direct member comparisons can be marked `constexpr` and
verified at compile time with `static_assert` under C++17, whereas `std::tie`
comparisons were not `constexpr` prior to C++20.

## Examples

Bad:

```cpp
#include <tuple>

inline bool operator<(const DriverVersion& a, const DriverVersion& b) {
    return std::tie(a.fMajor, a.fMinor) < std::tie(b.fMajor, b.fMinor);
}
```

Good:

```cpp
inline constexpr bool operator<(const DriverVersion& a, const DriverVersion& b) {
    return a.fMajor < b.fMajor || (a.fMajor == b.fMajor && a.fMinor < b.fMinor);
}

static_assert(DriverVersion(1, 0) < DriverVersion(1, 1));
```

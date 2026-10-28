---
name: skia/api/define-inverse-comparisons-via-counterparts
description: When defining inverse comparison operators such as operator!= or operator>=, implement them in terms of their canonical counterparts like !(a == b) rather than duplicating logic
rubric: readability
severity: should_fix
---

**Rule.** When defining overloaded inverse comparison operators (such as
operator!=, operator>=, or operator>), implement them by delegating directly to
their canonical counterparts (e.g., !(a == b), !(*this == other), or !(a < b))
rather than reimplementing independent comparison logic. Define canonical
equality and ordering operations first, and express complementary comparisons in
terms of them.

**Why.** Implementing inverse comparison operators in terms of their canonical
counterparts prevents logic divergence when comparison requirements change,
eliminates redundant member-by-member comparisons, and ensures consistency
across the codebase.

## Examples

Bad:

```cpp
inline bool operator==(const DriverVersion& a, const DriverVersion& b) {
    return a.fMajor == b.fMajor && a.fMinor == b.fMinor;
}
inline bool operator!=(const DriverVersion& a, const DriverVersion& b) {
    return a.fMajor != b.fMajor || a.fMinor != b.fMinor;
}
```

Good:

```cpp
inline constexpr bool operator==(const DriverVersion& a, const DriverVersion& b) {
    return a.fMajor == b.fMajor && a.fMinor == b.fMinor;
}
inline constexpr bool operator!=(const DriverVersion& a, const DriverVersion& b) {
    return !(a == b);
}
```

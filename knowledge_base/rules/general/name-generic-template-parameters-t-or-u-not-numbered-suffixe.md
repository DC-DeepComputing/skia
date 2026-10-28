---
name: skia/name-generic-template-parameters-t-or-u-not-numbered-suffixe
description: When declaring generic template parameters in helper traits or detection structs, use standard letters like T or U instead of numbered suffixes
rubric: readability
severity: nit
---

**Rule.** Do not use numbered suffixes on domain-specific names (e.g., `Traits1`
or `Type1`) to avoid shadowing when declaring generic template parameters in
nested helpers or detection traits. Use standard generic single-letter
identifiers such as `T` or `U` unless distinguishing multiple parallel types.

**Why.** In helper type traits and SFINAE/detection structs, the template
parameter represents any arbitrary type being inspected. Appending numeric
suffixes to domain names (such as `Traits1`) unnecessarily couples the generic
trait to an outer caller's domain and creates awkward names; standard
single-letter parameters like `T` or `U` clearly signal that the parameter is
generic and unconstrained.

## Examples

Bad:

```cpp
// Inside templated container SkTHashTable<T, Traits>:
template <typename Traits1, typename = void>
struct HasShouldGrow : std::false_type {};

template <typename Traits1>
struct HasShouldGrow<
        Traits1,
        std::void_t<decltype(Traits1::ShouldGrow(std::declval<int>(), std::declval<int>()))>>
        : std::true_type {};
```

Good:

```cpp
// Inside templated container SkTHashTable<T, Traits>:
template <typename U, typename = void>
struct HasShouldGrow : std::false_type {};

template <typename U>
struct HasShouldGrow<
        U,
        std::void_t<decltype(U::ShouldGrow(std::declval<int>(), std::declval<int>()))>>
        : std::true_type {};
```

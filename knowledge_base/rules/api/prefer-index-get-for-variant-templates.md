---
name: skia/api/prefer-index-get-for-variant-templates
description: When retrieving alternatives from a std::variant in generic templates, use index-based std::get<0> or assert distinct types with static_assert to avoid ambiguous lookups
rubric: correctness
severity: should_fix
---

**Rule.** When retrieving values from a std::variant within generic templates
(such as result or sum types), access alternatives by positional index
(std::get<0>, std::get<1>) rather than by type (std::get<T>), or explicitly
disallow duplicate types using static_assert(!std::is_same_v<T, E>).

**Why.** In C++, std::get<T> on a std::variant fails to compile if multiple
alternative types coincide (for example, if success and error types are
instantiated with the same type). Index-based access avoids ambiguity regardless
of template arguments, while a static_assert clearly documents and enforces the
distinct-type invariant at compile time.

## Examples

Bad:

```cpp
template <typename T, typename E>
class SkResult {
public:
    const T& value() const { return std::get<T>(fStorage); }
    const E& error() const { return std::get<E>(fStorage); }
private:
    std::variant<T, E> fStorage;
};
```

Good:

```cpp
template <typename T, typename E>
class SkResult {
    static_assert(!std::is_same_v<T, E>, "T and E must be different types.");
public:
    const T& value() const { return std::get<0>(fStorage); }
    const E& error() const { return std::get<1>(fStorage); }
private:
    std::variant<T, E> fStorage;
};
```

---
name: skia/prefer-braced-initializer-for-pair
description: When constructing or returning std::pair instances, prefer braced initializer lists {first, second} over explicit std::make_pair calls to reduce boilerplate
rubric: readability
severity: nit
---

**Rule.** Prefer braced initializer lists (`{first, second}`) over
`std::make_pair(...)` or explicit `std::pair<...>(...)` constructor calls when
returning, initializing, or passing `std::pair` instances. When the destination
type is known from a function return type, variable declaration, or container
signature, use braced initialization to minimize boilerplate and avoid
multi-line wrapping.

**Why.** Braced initialization `{a, b}` eliminates redundant `std::make_pair`
boilerplate and allows compact expressions to fit on a single line, following
modern C++ idioms across the codebase.

## Examples

Bad:

```cpp
// Bad: using std::make_pair adds verbose boilerplate and line wrapping
std::pair<sk_sp<SkImageFilter>, bool> processFilter(sk_sp<SkImageFilter> filter) {
    if (!filter) {
        return std::make_pair(nullptr, false);
    }
    return std::make_pair(
            SkImageFilters::Blend(SkBlendMode::kDstIn, std::move(filter), nullptr), false);
}
```

Good:

```cpp
// Good: braced initializer list {first, second} is concise and idiomatic
std::pair<sk_sp<SkImageFilter>, bool> processFilter(sk_sp<SkImageFilter> filter) {
    if (!filter) {
        return {nullptr, false};
    }
    return {SkImageFilters::Blend(SkBlendMode::kDstIn, std::move(filter), nullptr), false};
}
```

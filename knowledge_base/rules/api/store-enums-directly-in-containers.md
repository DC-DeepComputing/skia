---
name: skia/api/store-enums-directly-in-containers
description: When storing enum values in containers or collection members, use the strongly typed enum directly rather than raw integer types to avoid reinterpret_cast on SkSpan
rubric: readability
severity: should_fix
---

**Rule.** Store strongly typed enums directly as the element type in containers
and collection members (such as `skia_private::STArray`, `TArray`, or
`std::vector`) rather than their underlying integer type (such as `uint8_t`).
When exposing a span of elements via `SkSpan` or accessing elements, return or
pass the container directly without relying on `reinterpret_cast`.

**Why.** Storing the enum type directly preserves type safety across insertions,
indexing, and iteration. Storing raw integer types requires unsafe
`reinterpret_cast` when exposing `SkSpan` views or passing spans, bypassing
compile-time type checking.

## Examples

Bad:

```cpp
// Storing underlying integer type requires reinterpret_cast when exposing an SkSpan
using VerbsArray = skia_private::STArray<4, uint8_t>;
VerbsArray fVerbs;

SkSpan<const SkPathVerb> verbs() const {
    return {reinterpret_cast<const SkPathVerb*>(fVerbs.begin()), fVerbs.size()};
}
```

Good:

```cpp
// Storing the strongly typed enum directly preserves type safety and converts cleanly to SkSpan
using VerbsArray = skia_private::STArray<4, SkPathVerb>;
VerbsArray fVerbs;

SkSpan<const SkPathVerb> verbs() const {
    return fVerbs;
}
```

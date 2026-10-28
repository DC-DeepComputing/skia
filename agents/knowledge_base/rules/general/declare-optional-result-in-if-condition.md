---
name: skia/declare-optional-result-in-if-condition
description: When an optional or fallible result is used only inside a conditional block, declare it inline in the if condition and rely on contextual bool conversion
rubric: readability
severity: nit
---

**Rule.** When an optional or fallible lookup/computation is only needed within
a conditional block, declare and initialize the variable directly inside the
`if` condition (e.g. `if (auto result = ...)`) rather than declaring it in an
outer scope. Rely on `std::optional`'s or the smart pointer's contextual `bool`
conversion rather than calling `.has_value()` or comparing against
`std::nullopt`.

**Why.** Declaring the variable directly in the `if` statement confines its
scope and lifetime strictly to the branch where it is valid, preventing
accidental use of disengaged or uninitialized values outside the block. Relying
on contextual `bool` conversion avoids redundant member predicates like
`.has_value()` and follows idiomatic C++ conventions.

## Examples

Bad:

```cpp
auto result = Simplify(path);
if (result.has_value()) {
    return emscripten::val(result.value());
}

// Declared in outer scope before conditional check
std::optional<SkRect> bounds = computeBounds();
if (bounds != std::nullopt) {
    drawBounds(*bounds);
}
```

Good:

```cpp
if (auto result = Simplify(path)) {
    return emscripten::val(result.value());
}

// Scoped directly within condition using contextual bool conversion
if (auto bounds = computeBounds()) {
    drawBounds(*bounds);
}
```

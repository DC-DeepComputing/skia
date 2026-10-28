---
name: skia/prefer-has-value-over-nullopt-comparison-2
description: When checking whether a std::optional holds a value or is empty, call .has_value() or !opt.has_value() instead of comparing against std::nullopt
rubric: readability
severity: should_fix
---

**Rule.** When checking whether a std::optional<T> is engaged or empty, call
opt.has_value() (or !opt.has_value()) rather than comparing against std::nullopt
(such as opt == std::nullopt or opt != std::nullopt). Use member predicates
rather than equality comparisons to check optional presence.

**Why.** In Skia conventions, calling .has_value() or testing optional presence
directly expresses intent concisely and idiomatically. Avoiding equality
comparisons against std::nullopt maintains consistency with codebase style and
avoids unnecessary verbose syntax.

## Examples

Bad:

```cpp
if (offset == std::nullopt) {
    return {};
}
if (result != std::nullopt) {
    process(*result);
}
```

Good:

```cpp
if (!offset.has_value()) {
    return {};
}
if (result.has_value()) {
    process(*result);
}
```

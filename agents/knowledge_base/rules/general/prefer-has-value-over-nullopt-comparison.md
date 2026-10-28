---
name: skia/prefer-has-value-over-nullopt-comparison
description: When checking whether a std::optional holds a value, call .has_value() instead of comparing against std::nullopt, and avoid creating temporary variables for immediate presence checks
rubric: readability
severity: nit
---

**Rule.** When checking whether a std::optional holds a value or is
empty/disengaged, call opt.has_value() or !opt.has_value() rather than comparing
against std::nullopt (== std::nullopt or != std::nullopt). Furthermore, when
evaluating whether a function returning std::optional produced a value, call
.has_value() directly on the returned expression instead of storing the result
in an intermediate named variable solely to inspect presence.

**Why.** Calling .has_value() directly queries the optional's state through its
standard member function, avoiding the syntactic verbosity of std::nullopt
comparisons and matching Skia codebase conventions. Chaining .has_value()
directly onto a function call avoids unnecessary local variable boilerplate when
the contained value is not needed.

## Examples

Bad:

```cpp
// Comparing optional against std::nullopt
const bool overflowed = offset == std::nullopt;
if (result != std::nullopt) {
    process(*result);
}

// Redundant temporary variable solely to check presence
std::optional<uint32_t> fit = can_offset_fit(bytes, size, offset, align, stride);
return fit == std::nullopt;
```

Good:

```cpp
// Use .has_value() or !opt.has_value()
const bool overflowed = !offset.has_value();
if (result.has_value()) {
    process(*result);
}

// Call .has_value() directly on the returned optional
return can_offset_fit(bytes, size, offset, align, stride).has_value();
```

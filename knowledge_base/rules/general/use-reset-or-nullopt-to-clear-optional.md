---
name: skia/use-reset-or-nullopt-to-clear-optional
description: When returning or clearing an empty std::optional, return std::nullopt or call .reset() instead of using empty braces {}
rubric: readability
severity: nit
---

**Rule.** When returning an empty std::optional from a function or clearing an
existing std::optional variable, use std::nullopt (or call .reset()) rather than
empty braces {}. Do not write return {}; or assign opt = {}; to represent an
unengaged or disengaged optional.

**Why.** Using std::nullopt or .reset() explicitly conveys that the optional
contains no value. In contrast, empty braces ({}) are ambiguous and less
readable because braced initialization can be confused with default-constructing
or value-initializing the wrapped inner type.

## Examples

Bad:

```cpp
// Returning an empty optional:
std::optional<SkPath> Op(const SkPath& one, const SkPath& two, SkPathOp op) {
    SkPath result;
    if (skpathopsinternal::Op(one, two, op, &result)) {
        return result;
    }
    return {};
}

// Clearing an existing optional:
if (!validate(bytesRead > 0)) {
    path = {};
}
```

Good:

```cpp
// Returning an empty optional:
std::optional<SkPath> Op(const SkPath& one, const SkPath& two, SkPathOp op) {
    SkPath result;
    if (skpathopsinternal::Op(one, two, op, &result)) {
        return result;
    }
    return std::nullopt;
}

// Clearing an existing optional:
if (!validate(bytesRead > 0)) {
    path.reset(); // or: path = std::nullopt;
}
```

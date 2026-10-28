---
name: skia/name-peer-parameters-that
description: When naming peer parameters in copy or move constructors, assignment operators, or comparison operations, name the parameter that rather than other or args
rubric: readability
severity: nit
---

**Rule.** When naming the parameter representing the peer or source instance in
copy or move constructors, assignment operators, or comparison operations (such
as operator== or onIsEqual), name the parameter that. Do not use generic
parameter names like other, args, rhs, or single-letter abbreviations.

**Why.** Skia consistently pairs this and that across the codebase for
operations comparing or transferring state from a peer instance of the same
type. Following this naming convention maintains codebase consistency and
clarifies intent across declarations and definitions.

## Examples

Bad:

```cpp
class FontArguments {
public:
    FontArguments(const FontArguments& args);
    FontArguments& operator=(const FontArguments& other);
    bool operator==(const FontArguments& rhs) const;
};
```

Good:

```cpp
class FontArguments {
public:
    FontArguments(const FontArguments& that);
    FontArguments& operator=(const FontArguments& that);
    bool operator==(const FontArguments& that) const;
};
```

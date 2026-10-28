---
name: skia/api/prefer-enum-class-over-wrapper-struct
description: When defining simple enumerations without additional fields or methods, prefer scoped enum class over wrapper structs or classes with inner enums
rubric: readability
severity: should_fix
---

**Rule.** Prefer scoped enums (enum class) over wrapper structs or classes when
defining enumeration types that have no additional member fields or methods. Do
not wrap an enum inside a struct or class with an inner enum Type and an fType
field merely to achieve scoping or provide conversion constructors; define the
type directly as a scoped enum class.

**Why.** In modern C++, scoped enums (enum class) provide type safety, scope
isolation, and explicit underlying storage directly. Wrapping an enum in a
struct or class introduces unnecessary boilerplate in type definitions,
constructors, and call-site member access (such as val.fType or val->fType)
without adding any encapsulation or functionality.

## Examples

Bad:

```cpp
struct SkSVGDisplay {
    enum Type {
        kInline,
        kNone,
    };

    Type fType;

    SkSVGDisplay() : fType(kInline) {}
    explicit SkSVGDisplay(Type type) : fType(type) {}
};
```

Good:

```cpp
enum class SkSVGDisplay {
    kInline,
    kNone,
};
```

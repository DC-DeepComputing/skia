---
name: skia/api/avoid-underscore-type-namespacing
description: When defining families of related types or helper shapes, scope them in a dedicated namespace or enclosing class instead of using underscore pseudo-namespacing
rubric: readability
severity: nit
---

**Rule.** Scope related types and shape variants inside a dedicated namespace
(e.g., namespace SkPathRawShapes) or as nested structs or classes within an
enclosing type, rather than using underscore delimiters in type names (such as
SkPathRawShape_Rect) to emulate namespacing. Do not introduce new
pseudo-namespaced type names that mimic legacy Skia underscore naming patterns.

**Why.** Using underscore separators to emulate namespacing in type names is a
legacy C idiom that pollutes the enclosing scope with long symbol names. Modern
C++ namespaces and nested structs provide proper modular scoping, eliminate
redundant type prefixes, and avoid collision hazards without awkward underscore
delimiters.

## Examples

Bad:

```cpp
// Bad: Using underscore delimiters to emulate namespacing for related types
struct SkPathRawShape_Rect : public SkPathRaw {
    SkPoint fStorage[4];
    explicit SkPathRawShape_Rect(const SkRect&);
};

struct SkPathRawShape_Oval : public SkPathRaw {
    SkPoint fStorage[9];
    explicit SkPathRawShape_Oval(const SkRect&);
};
```

Good:

```cpp
// Good: Scoping related types inside a dedicated namespace
namespace SkPathRawShapes {

struct Rect : public SkPathRaw {
    SkPoint fStorage[4];
    explicit Rect(const SkRect&);
};

struct Oval : public SkPathRaw {
    SkPoint fStorage[9];
    explicit Oval(const SkRect&);
};

}  // namespace SkPathRawShapes
```

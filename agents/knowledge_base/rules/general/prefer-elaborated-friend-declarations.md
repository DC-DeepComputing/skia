---
name: skia/prefer-elaborated-friend-declarations
description: When befriending a class in C++ headers, use elaborated friend declarations (`friend class ClassName;`) instead of bare `friend ClassName;` to avoid redundant forward declarations
rubric: readability
severity: nit
---

**Rule.** When granting friendship to a class or struct, use an elaborated
friend declaration (`friend class ClassName;` or `friend struct StructName;`)
rather than a bare friend declaration (`friend ClassName;`). Avoid preceding the
class with a separate forward declaration when the type is only referenced in
friend declarations.

**Why.** Bare friend declarations (`friend ClassName;`) require the named type
to already be declared in scope, necessitating an extra forward declaration.
Elaborated friend declarations (`friend class ClassName;`) introduce the type
into the enclosing namespace if not already declared, keeping headers clean and
avoiding redundant forward declarations.

## Examples

Bad:

```cpp
// Redundant forward declaration needed only for bare friend declaration
class SkSurface_Base;

class SkRecorder {
    ...
    friend SkSurface_Base;
};
```

Good:

```cpp
class SkRecorder {
    ...
    // Elaborated friend declaration introduces the class without a separate forward declaration
    friend class SkSurface_Base;
};
```

---
name: skia/cite-specific-header-in-comments
description: When referencing declarations, enums, or invariants in implementation comments, cite the specific header file name rather than using vague shorthand like .h or the header
rubric: readability
severity: nit
---

**Rule.** When referencing definitions, enums, or invariants declared in a
header within implementation comments, explicitly cite the specific header file
(e.g., 'Foo.h') instead of ambiguous shorthand such as 'defined in .h' or 'see
the header'.

**Why.** Source files typically include numerous headers. Vague shorthand like
'.h' or 'the header' forces readers to search across multiple included files to
locate declarations or verify invariant assumptions, whereas naming the concrete
header file directly points maintainers to the source of truth.

## Examples

Bad:

```cpp
// defined in .h, assumes contiguous enum ending at kLast
return static_cast<ShaderType>(rand->nextULessThan(static_cast<int>(ShaderType::kLast) + 1));
```

Good:

```cpp
// defined in PaintParamsTestUtils.h, assumes contiguous enum ending at kLast
return static_cast<ShaderType>(rand->nextULessThan(static_cast<int>(ShaderType::kLast) + 1));
```

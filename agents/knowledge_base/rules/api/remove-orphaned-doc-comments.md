---
name: skia/api/remove-orphaned-doc-comments
description: When removing, renaming, or refactoring methods or member variables, remove or relocate preceding doc comments so they do not linger above unrelated declarations
rubric: readability
severity: nit
---

**Rule.** When removing, renaming, or refactoring methods, functions, or member
variables, remove or relocate any preceding documentation comments that
described the altered declaration. Do not leave orphaned or misplaced doc
comments sitting above adjacent, unrelated declarations where they misrepresent
the adjacent code's semantics.

**Why.** Doc comments left behind after deleting or refactoring an adjacent
member contradict the semantics of whatever declaration happens to follow them.
This confuses reviewers, API consumers, and future maintainers about what the
remaining method or field actually does.

## Examples

Bad:

```cpp
// Indicates if any paint uniforms are needed by expressions evaluated in the vertex shader
bool hasGradientBuffer() const { return fHasGradientBuffer; }
```

Good:

```cpp
bool hasGradientBuffer() const { return fHasGradientBuffer; }
```

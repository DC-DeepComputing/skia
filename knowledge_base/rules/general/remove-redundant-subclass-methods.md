---
name: skia/remove-redundant-subclass-methods
description: When moving or devirtualizing member functions into a base class, delete redundant subclass method implementations rather than stripping the override specifier
rubric: readability
severity: should_fix
---

**Rule.** When moving or devirtualizing a member function into a common base
class, remove the redundant method declarations and implementations from all
derived subclasses. Do not merely strip the override specifier or retain
duplicate shadowing methods in subclasses to silence compiler errors.

**Why.** Leaving redundant subclass methods after lifting or devirtualizing
behavior into a base class clutters derived class definitions with dead
shadowing code. Derived classes should inherit the common implementation
directly from the base class, keeping class definitions concise and
single-sourced.

## Examples

Bad:

```cpp
// When vertexFiller() is moved into the AtlasSubRun base class:
class SDFTSubRun final : public AtlasSubRun {
    // Bad: 'override' was removed to silence compiler, leaving a redundant shadow method
    const VertexFiller& vertexFiller() const { return fVertexFiller; }
};
```

Good:

```cpp
// When vertexFiller() is moved into the AtlasSubRun base class:
class SDFTSubRun final : public AtlasSubRun {
    // Good: redundant method removed; inherited directly from AtlasSubRun
};
```

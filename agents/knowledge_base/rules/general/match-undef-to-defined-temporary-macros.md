---
name: skia/match-undef-to-defined-temporary-macros
description: 'When defining temporary helper macros for X-macro expansions or local tables, ensure every defined macro identifier is paired with an exact matching #undef'
rubric: readability
severity: should_fix
---

**Rule.** When defining temporary helper macros (such as expansion macros for
X-macro lists, enum generators, or local lookup tables), ensure every defined
macro identifier is paired with a matching #undef directive after its use.
Verify that #undef names match the defined identifiers exactly, especially
during refactoring or renaming, so un-prefixed helper macros do not leak into
subsequent code.

**Why.** Preprocessor macros lack C++ namespace scoping and leak across include
boundaries or subsequent code in translation units if not undefined. Because
#undef on an undefined identifier silently succeeds without compiler warnings, a
typo or stale macro name in an #undef directive leaves the intended helper macro
defined, risking unintended macro substitution and global name collisions.

## Examples

Bad:

```cpp
#define ENUM1(BaseName) k##BaseName,
#define ENUM3(BaseName, VariantName, VariantTag) k##BaseName##_##VariantName,
enum class RenderStepID : uint32_t {
    SKGPU_RENDERSTEP_TYPES(ENUM1, ENUM3)
};
#undef ENUM1
#undef ENUM2  // Typo: ENUM3 remains defined and leaks into subsequent code
```

Good:

```cpp
#define ENUM1(BaseName) k##BaseName,
#define ENUM3(BaseName, VariantName, VariantTag) k##BaseName##_##VariantName,
enum class RenderStepID : uint32_t {
    SKGPU_RENDERSTEP_TYPES(ENUM1, ENUM3)
};
#undef ENUM1
#undef ENUM3
```

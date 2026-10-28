---
name: skia/api/document-planned-enum-values-with-todo
description: When declaring enum values or API hooks reserved for planned features, annotate them with an attributed TODO comment rather than leaving them undocumented
rubric: readability
severity: nit
---

**Rule.** When declaring enum values, flags, or architectural hooks that are
reserved for planned future features and currently unreferenced, annotate them
with an explanatory TODO(b/...) or TODO(skbug.com/...) comment referencing a
tracking issue and describing the planned usage. Do not delete them as dead code
if they establish intentional API design, and do not leave them undocumented
without an attributed tracking issue.

**Why.** Unreferenced enum values and architectural hooks are easily mistaken
for dead code or abandoned prototypes during review and refactoring. Documenting
them with an attributed TODO comment preserves design intent, clarifies why the
value exists ahead of implementation, and prevents accidental deletion in
cleanups.

## Examples

Bad:

```cpp
enum class DstReadStrategy : uint8_t {
    kNoneRequired,
    kTextureCopy,
    kTextureSample,  // Unused in current CL; easily mistaken for dead code
    kReadFromInput,
    kFramebufferFetch,
};
```

Good:

```cpp
enum class DstReadStrategy : uint8_t {
    kNoneRequired,
    kTextureCopy,
    kTextureSample,  // TODO(b/238756862): To be used once direct texture sampling is implemented
    kReadFromInput,
    kFramebufferFetch,
};
```

---
name: skia/align-macro-table-columns
description: When defining or updating multi-line macro tables with multiple arguments, vertically align parameter columns across lines and align trailing line-continuation backslashes
rubric: readability
severity: nit
---

**Rule.** When defining or updating multi-line macro tables (X-macros) with
multiple arguments, vertically align parameter columns across lines so the
arguments form uniform columns. Keep trailing line-continuation backslashes
aligned across all lines in the macro definition.

**Why.** Code formatters do not automatically align macro arguments across
calls. Manually aligning parameter columns and trailing line-continuation
backslashes makes large tabular macro definitions easier to scan, verify, and
diff.

## Examples

Bad:

```cpp
#define SKGPU_RENDERSTEP_TYPES(M1, M2) \
    M1(Invalid) \
    M2(CoverBounds, NonAAFill) \
    M2(BitmapText, Mask) \
    M2(BitmapText, LCD)
```

Good:

```cpp
#define SKGPU_RENDERSTEP_TYPES(M1, M2)              \
    M1(Invalid)                                     \
    M2(CoverBounds,      NonAAFill)                 \
    M2(BitmapText,       Mask)                      \
    M2(BitmapText,       LCD)
```

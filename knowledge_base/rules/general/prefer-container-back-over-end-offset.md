---
name: skia/prefer-container-back-over-end-offset
description: When accessing the last element of a container or span, use the standard back() accessor instead of manual iterator offset indexing like end()[-1]
rubric: readability
severity: nit
---

**Rule.** When accessing the trailing element of a container or span (such as
std::vector, skia_private::TArray, or SkSpan), use the standard .back()
accessor. Do not use manual iterator negative indexing or pointer offset
arithmetic such as container.end()[-1] or *(container.end() - 1).

**Why.** Standard container accessors like .back() clearly convey the intent of
retrieving the final element and produce idiomatic C++. In contrast, indexing
past-the-end iterators with negative offsets like end()[-1] is obscure,
non-standard, and adds unnecessary iterator syntax.

## Examples

Bad:

```cpp
// Non-idiomatic negative iterator indexing
SkASSERT(!fPathVerbs.empty() && fPathVerbs.end()[-1] == SkPath::kDone_Verb);
```

Good:

```cpp
// Idiomatic standard container accessor
SkASSERT(!fPathVerbs.empty() && fPathVerbs.back() == SkPath::kDone_Verb);
```

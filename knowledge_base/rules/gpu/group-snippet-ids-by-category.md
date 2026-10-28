---
name: skia/gpu/group-snippet-ids-by-category
description: When declaring code snippet IDs in Graphite enums, group base snippets and their specializations together under dedicated functional category comments
rubric: readability
severity: should_fix
---

**Rule.** In Graphite code snippet ID enums (such as BuiltInCodeSnippetID),
group base snippets and their specializations together under dedicated
functional category comments rather than leaving related or redundant snippets
scattered across general sections. When a snippet is suspected to be redundant
with a specialization, document it with an attributed TODO comment.

**Why.** Graphite's built-in code snippets are categorized by functionality.
Grouping base snippets and their specializations together under clear category
headers makes snippet relationships and potential redundancies obvious, making
it easier to track follow-up cleanups and consolidations.

## Examples

Bad:

```cpp
    // SkColorFilter code snippets
    kMatrixColorFilter,
    kTableColorFilter,
    kGaussianColorFilter,
    kColorSpaceXformColorFilter,
    kPremulAlphaColorFilter,

    // Color space transform specializations
    kColorSpaceXformPremul,
```

Good:

```cpp
    // SkColorFilter code snippets
    kMatrixColorFilter,
    kTableColorFilter,
    kGaussianColorFilter,

    // Color space transform snippet and its specializations
    kColorSpaceXformColorFilter,
    kColorSpaceXformPremul,
    // TODO(b/388849244): This seems redundant with kColorSpaceXformPremul
    kPremulAlphaColorFilter,
```

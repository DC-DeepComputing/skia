---
name: skia/general-use-full-gerrit-urls-in-comments
description: When citing past Gerrit changes or code reviews in comments, provide the full clickable URL rather than a bare CL number
rubric: readability
severity: nit
---

**Rule.** When referencing or citing a Gerrit change or code review in comments,
provide the full clickable URL (e.g.,
https://skia-review.googlesource.com/c/skia/+/<cl_number>) rather than a bare CL
number or change ID. If citing an external Chromium or Git change, provide its
full review URL or repository link.

**Why.** Bare Gerrit change numbers in comments are ambiguous and not clickable
in code review tools, IDEs, or terminals. Providing the full URL allows future
maintainers and reviewers to directly navigate to the change's discussion,
diffs, and context during git archaeology.

## Examples

Bad:

```cpp
// Attempted to solve this ordering issue by adding a variable epsilon in 1004396, but this had to
// be removed as it caused regressions in the fuzzer.
```

Good:

```cpp
// Attempted to solve this ordering issue by adding a variable epsilon in
// https://skia-review.googlesource.com/c/skia/+/1004396, but this had to be removed as it caused
// regressions in the fuzzer.
```

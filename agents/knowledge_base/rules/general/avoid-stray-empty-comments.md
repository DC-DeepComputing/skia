---
name: skia/avoid-stray-empty-comments
description: When writing comments and code, do not leave stray '//' markers on statements or trailing empty '//' lines ending comment blocks
rubric: readability
severity: nit
---

**Rule.** Do not leave stray empty comment markers (such as trailing '//' or
empty '/* */' blocks) or leftover touch edits on statements or declarations.
Additionally, do not end a comment block with a trailing empty comment line
('//'); separate the comment block from subsequent code statements with a blank
line instead.

**Why.** Empty comment delimiters or dangling '//' lines add visual clutter,
convey no explanatory value, and confuse readers by suggesting that an
intentional note or code snippet was accidentally omitted. Multi-line comment
blocks should be cleanly separated from subsequent code with blank lines rather
than trailing comment delimiters.

## Examples

Bad:

```cpp
static constexpr const RunHead* const kRectRunHeadPtr = nullptr;//

// * frontFace
// * rasterizerDiscardEnable
//
if (fSharedContext->caps()->useBasicDynamicState()) {
```

Good:

```cpp
static constexpr const RunHead* const kRectRunHeadPtr = nullptr;

// * frontFace
// * rasterizerDiscardEnable

if (fSharedContext->caps()->useBasicDynamicState()) {
```

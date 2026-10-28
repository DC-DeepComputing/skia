---
name: skia/release_notes-md/do-not-edit-release-notes-directly
description: When documenting changes or updating release notes, author files in relnotes/ without leading bullet asterisks and avoid direct edits or duplicate bullets in RELEASE_NOTES.md
rubric: readability
severity: should_fix
---

**Rule.** Do not edit RELEASE_NOTES.md directly for new features, bug fixes, or
API changes; add individual Markdown (*.md) files under relnotes/ instead. When
authoring notes in relnotes/ or updating milestone sections in RELEASE_NOTES.md,
do not prefix entries with asterisks, hyphens, or list markers, and verify that
aggregated notes do not contain duplicate bullet marks ('* *').

**Why.** Skia's release automation aggregates individual files from relnotes/
into RELEASE_NOTES.md during milestone releases and automatically inserts an
unordered list bullet (' * ') before each note. Editing RELEASE_NOTES.md
directly bypasses release tooling and causes branch merge conflicts, while
starting notes with an asterisk or list marker causes malformed duplicate bullet
points (' * * ') in published release notes.

## Examples

Bad:

```cpp
<!-- RELEASE_NOTES.md: direct edit instead of relnotes/ file -->
Milestone 148
-------------
  * Virtuals in SkTypeface subclasses now take SkSpan.

<!-- relnotes/texture_wrap.md: note starts with leading asterisk bullet -->
* `SkSurfaces::WrapBackendTexture` no longer requires providing an `SkColorType`.

<!-- RELEASE_NOTES.md: duplicate bullet points from aggregated leading asterisk -->
  * * `SkSurfaces::WrapBackendTexture` no longer requires providing an `SkColorType`.
```

Good:

```cpp
<!-- relnotes/typeface_virtuals.md: individual note without leading bullet -->
Virtuals in `SkTypeface` subclasses now take `SkSpan` instead of pointer and count.

<!-- relnotes/texture_wrap.md: plain markdown text without leading asterisk -->
`SkSurfaces::WrapBackendTexture` no longer requires providing an `SkColorType`.

<!-- RELEASE_NOTES.md: aggregated output with clean single bullet -->
  * `SkSurfaces::WrapBackendTexture` no longer requires providing an `SkColorType`.
```

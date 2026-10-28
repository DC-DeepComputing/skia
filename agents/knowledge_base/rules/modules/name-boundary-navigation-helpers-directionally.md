---
name: skia/modules/name-boundary-navigation-helpers-directionally
description: When declaring boundary navigation helpers in text layout, use explicit findNext<Unit>Boundary and findPrevious<Unit>Boundary names rather than ambiguous Start or End suffixes
rubric: readability
severity: should_fix
---

**Rule.** Name text and sequence boundary navigation helpers with explicit
directional prefixes and target boundary nouns (such as findNext<Unit>Boundary
and findPrevious<Unit>Boundary, e.g., findNextGraphemeBoundary or
findPreviousGlyphClusterBoundary). Avoid ambiguous Start or End suffixes (such
as findGraphemeStart or findGraphemeEnd) that conflate scan direction with
segment boundary.

**Why.** In text segmentation (such as graphemes, glyph clusters, or words),
each boundary simultaneously marks the end of one segment and the start of the
next. Suffixes like Start or End create ambiguity about whether the name refers
to the scan traversal direction or the segment boundary being located. Explicit
findNext...Boundary and findPrevious...Boundary prefixes make both the search
direction and boundary target clear at declarations and call sites.

## Examples

Bad:

```cpp
// Ambiguous whether 'End'/'Start' indicates scan direction or segment boundary
TextIndex ParagraphImpl::findGraphemeEnd(TextIndex utf8) {
    while (utf8 > 0 &&
          (fCodeUnitProperties[utf8] & CodeUnitFlags::kGraphemeStart) == 0) {
        --utf8;
    }
    return utf8;
}

TextIndex ParagraphImpl::findGraphemeStart(TextIndex utf8) {
    while (utf8 < fText.size() &&
          (fCodeUnitProperties[utf8] & CodeUnitFlags::kGraphemeStart) == 0) {
        ++utf8;
    }
    return utf8;
}
```

Good:

```cpp
// Explicit directional prefixes and boundary target clarify traversal
TextIndex ParagraphImpl::findPreviousGraphemeBoundary(TextIndex utf8) {
    while (utf8 > 0 &&
          (fCodeUnitProperties[utf8] & CodeUnitFlags::kGraphemeStart) == 0) {
        --utf8;
    }
    return utf8;
}

TextIndex ParagraphImpl::findNextGraphemeBoundary(TextIndex utf8) {
    while (utf8 < fText.size() &&
          (fCodeUnitProperties[utf8] & CodeUnitFlags::kGraphemeStart) == 0) {
        ++utf8;
    }
    return utf8;
}
```

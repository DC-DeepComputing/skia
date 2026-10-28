---
name: skia/modules/name-directional-boundary-searches-with-previous-next
description: When defining directional boundary search functions in text layout, name methods with findPrevious/findNext and Boundary rather than Start/End
rubric: readability
severity: should_fix
---

**Rule.** When defining directional boundary search functions in text layout
(such as for graphemes, glyph clusters, words, or line breaks), name methods
with explicit traversal directions (findPrevious... / findNext...) and a
boundary descriptor (...Boundary) rather than misleading or asymmetric Start/End
suffixes. In particular, do not name a backward scan with an End suffix (such as
findGraphemeEnd) or a forward scan with a Start suffix.

**Why.** Boundary positions in text can be traversed in both directions, and a
backward scan locates a preceding boundary rather than an 'end'. Using Start and
End for directional searches is confusing and easily inverted, whereas Previous
and Next paired with Boundary unambiguously communicate both the traversal
direction and the boundary condition being resolved.

## Examples

Bad:

```cpp
// Searching backward to locate preceding grapheme boundary
TextIndex ParagraphImpl::findGraphemeEnd(TextIndex utf8) const {
    while (utf8 > 0 &&
          (fCodeUnitProperties[utf8] & CodeUnitFlags::kGraphemeStart) == 0) {
        --utf8;
    }
    return utf8;
}
```

Good:

```cpp
// Searching backward to locate preceding grapheme boundary
TextIndex ParagraphImpl::findPreviousGraphemeBoundary(TextIndex utf8) const {
    while (utf8 > 0 &&
          (fCodeUnitProperties[utf8] & CodeUnitFlags::kGraphemeStart) == 0) {
        --utf8;
    }
    return utf8;
}
```

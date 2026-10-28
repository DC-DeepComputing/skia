---
name: skia/modules/avoid-manual-utf8-adjustment-after-grapheme-boundary
description: When resolving text range boundaries using grapheme cluster helpers in SkParagraph, do not manually increment or decrement raw UTF-8 byte offsets
rubric: testing
severity: should_fix
---

**Rule.** Do not manually increment or decrement raw UTF-8 byte offsets after
resolving a boundary via helper functions like findPreviousGraphemeBoundary or
findNextGraphemeBoundary. Grapheme cluster boundaries already align with Unicode
code point boundaries and point to leading UTF-8 code units; any adjustments for
surrogate pairs belong on input indices prior to conversion.

**Why.** In UTF-8, multi-byte code points consist of a leading byte followed by
continuation bytes. Incrementing or decrementing a raw byte offset without
decoding risks pointing into the middle of a multi-byte sequence, producing
malformed UTF-8 offsets that corrupt downstream text slicing and layout range
queries.

## Examples

Bad:

```cpp
auto utf8 = findPreviousGraphemeBoundary(fUTF8IndexForUTF16Index[end]);
if (utf8 > 0 && fUTF16IndexForUTF8Index[utf8 - 1] == fUTF16IndexForUTF8Index[utf8]) {
    --utf8;
}
text.end = utf8;
```

Good:

```cpp
auto utf8 = findPreviousGraphemeBoundary(fUTF8IndexForUTF16Index[end]);
text.end = utf8;
```

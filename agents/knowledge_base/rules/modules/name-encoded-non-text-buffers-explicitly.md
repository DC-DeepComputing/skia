---
name: skia/modules/name-encoded-non-text-buffers-explicitly
description: When encoding non-textual identifiers or raw data into text or UTF-8 buffers, name the buffer explicitly after the semantic payload rather than generic format names
rubric: readability
severity: nit
---

**Rule.** When encoding non-textual data, internal IDs, or non-Unicode values
(such as glyph IDs) into a text or string format like UTF-8, name the buffer
explicitly after its semantic payload (e.g., `glyphid_as_utf8` or
`<concept>_as_<format>`) rather than using generic format names like `utf8` or
`str`.

**Why.** Naming a buffer generically after its encoding format misleadingly
implies that it holds standard Unicode characters or human-readable text. Naming
the variable after its semantic payload makes clear that an internal identifier
is being packed into the text container.

## Examples

Bad:

```cpp
// glyph.fID is an internal glyph ID being encoded into a UTF-8 character field
char utf8[SkUTF::kMaxBytesInUTF8Sequence];
size_t utf8_len = SkUTF::ToUTF8(glyph.fID, utf8);
```

Good:

```cpp
char glyphid_as_utf8[SkUTF::kMaxBytesInUTF8Sequence];
size_t utf8_len = SkUTF::ToUTF8(glyph.fID, glyphid_as_utf8);
```

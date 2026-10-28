---
name: skia/document-text-index-encoding-and-units
description: When documenting public APIs that accept string indices or text offsets, explicitly specify the expected character encoding, unit representation, and standard platform sources
rubric: readability
severity: nit
---

**Rule.** When documenting public functions, methods, or interface parameters
that accept string indices, character boundaries, or text offsets, explicitly
specify the expected character encoding and unit representation (such as UTF-16
code units, UTF-8 byte offsets, or Unicode codepoints). Clarify whether indices
are 0-based and relative to the full text, and reference standard platform data
sources or segmentation APIs (such as `Intl.Segmenter`) where applicable. Do not
leave index parameters vaguely described without stating their unit and encoding
contract.

**Why.** Different environments, languages, and subsystems represent text using
disparate units and encodings (e.g., JavaScript strings and Web APIs use UTF-16
code units, while internal C++ engines often use UTF-8 byte offsets). Omitting
explicit encoding and unit specifications causes callers to supply mismatched
indices, resulting in incorrect text slicing, broken grapheme/word boundaries,
or crashes on multi-byte characters and surrogate pairs.

## Examples

Bad:

```cpp
/**
 * @param graphemes is an array of indexes in the input text that point
 * to the start of each grapheme.
 */
buildWithClientInfo(bidiRegions?: InputBidiRegions | null,
                    words?: InputWords | null,
                    graphemes?: InputGraphemes | null,
                    lineBreaks?: InputLineBreaks | null): Paragraph;
```

Good:

```cpp
/**
 * @param graphemes is an array of indexes in the input text that point
 * to the start of each grapheme.
 *
 * The indices are expected to be relative to the UTF-16 representation of
 * the text.
 *
 * The `Intl.Segmenter` API can be used as a source for this data.
 */
buildWithClientInfo(bidiRegions?: InputBidiRegions | null,
                    words?: InputWords | null,
                    graphemes?: InputGraphemes | null,
                    lineBreaks?: InputLineBreaks | null): Paragraph;
```

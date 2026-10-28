---
name: skia/document-packed-data-in-examples
description: When writing API examples or demo code that pass synthetic or packed data arrays, document the layout format and simplifying assumptions
rubric: readability
severity: nit
---

**Rule.** In API examples, demo pages, and sample code that pass manually
constructed or packed low-level data structures (such as text segmentation
arrays, offset spans, or raw byte buffers), document the data layout format and
parameter semantics in accompanying comments. Additionally, state any
simplifying assumptions (such as ASCII-only text, fixed reading direction, or
uniform glyph mapping) so consumers understand the constraints of the
demonstration.

**Why.** Sample code and demos serve as reference material for external clients
and reviewers. Passing raw packed numeric arrays or low-level buffers without
explanation leaves magic offsets and format expectations opaque, making the code
difficult to adapt and risking incorrect API usage.

## Examples

Bad:

```cpp
// Undocumented packed numeric arrays in demo code
const mallocedWords = CanvasKit.Malloc(Uint32Array, 2);
mallocedWords.toTypedArray().set([0, text.length]);

const mallocedBidis = CanvasKit.Malloc(Uint32Array, 3);
mallocedBidis.toTypedArray().set([0, text.length, 0]);

builder.setWordsUtf16(mallocedWords);
const paragraph = builder.build();
```

Good:

```cpp
// Demo assumes English-only LTR text where each character is a single glyph.
// Pass the entire text as one word: [start, end) offset pair.
const mallocedWords = CanvasKit.Malloc(Uint32Array, 2);
mallocedWords.toTypedArray().set([0, text.length]);

// Pass 1 bidi region as a triple: [start, end) offset pair and direction (0 = LTR).
const mallocedBidis = CanvasKit.Malloc(Uint32Array, 3);
mallocedBidis.toTypedArray().set([0, text.length, 0]);

builder.setWordsUtf16(mallocedWords);
const paragraph = builder.build();
```

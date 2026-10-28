---
name: skia/general-document-flattened-array-schemas
description: When declaring APIs accepting, or code populating, packed or flattened numeric array buffers, document the tuple layout schema, element strides, and value encodings
rubric: readability
severity: should_fix
---

**Rule.** When declaring API or binding methods that accept packed, structured,
or flattened array buffers (e.g., TypedArrays or raw memory allocations),
document the expected buffer layout, element stride or grouping (such as tuples
or triples), value encodings, and index bounds in parameter doc comments.
Likewise, when passing or populating flattened numeric arrays or buffers in call
sites or test fixtures, document the tuple schema and mock value intent in
inline comments so callers and maintainers understand the logical structure
without reverse-engineering consuming decoders.

**Why.** Flat or packed primitive arrays discard semantic field names and type
definitions, obscuring structured data behind raw numeric sequences. Without
explicit documentation of element strides, boundary conventions (inclusive vs.
exclusive), and value encodings, API callers cannot construct valid input
buffers and maintainers cannot verify mock test data against expected
invariants.

## Examples

Bad:

```cpp
// API declaration missing stride, tuple grouping, and encoding docs
buildWithClientInfo(bidiRegions?: Uint32Array | null,
                    words?: Uint32Array | null): Paragraph;

// Populating flattened arrays in test setup without schema comments
const bidis = CanvasKit.Malloc(Uint32Array, 3);
bidis.toTypedArray().set([0, text.length, 0]);
```

Good:

```cpp
/**
 * @param bidiRegions Array of uint32 triples: [start, end, direction] where
 *                    direction 0 is left-to-right and 1 is right-to-left.
 * @param words Array of word boundary edges [start, end) in the input text.
 */
buildWithClientInfo(bidiRegions?: Uint32Array | null,
                    words?: Uint32Array | null): Paragraph;

// Pass 1 bidi region as a triple: [start, end) offset pair and direction (0 = LTR)
const bidis = CanvasKit.Malloc(Uint32Array, 3);
bidis.toTypedArray().set([0, text.length, 0]);
```

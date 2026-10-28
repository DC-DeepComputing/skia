---
name: skia/modules/prefer-standard-arrays-over-canvaskit-malloc
description: When passing array data in CanvasKit, prefer standard JS arrays over CanvasKit.Malloc, and ensure any CanvasKit.Malloc allocation is released with CanvasKit.Free
rubric: safety
severity: should_fix
---

**Rule.** When passing array data to CanvasKit APIs (such as ParagraphBuilder
break methods, client info, or color/matrix helpers), prefer passing standard
JavaScript arrays or TypedArrays directly instead of allocating memory with
CanvasKit.Malloc. If CanvasKit.Malloc is used—such as for performance-critical
reuse across frames or in tests—ensure every allocated buffer is explicitly
released via CanvasKit.Free once no longer needed.

**Why.** CanvasKit's internal marshalling automatically allocates and
deallocates WebAssembly linear memory for standard JavaScript arrays and
TypedArrays. In contrast, CanvasKit.Malloc allocates memory directly on the
WebAssembly linear heap outside the scope of the JavaScript garbage collector,
resulting in permanent WASM heap memory leaks if CanvasKit.Free is omitted.

## Examples

Bad:

```cpp
// Case 1: Unnecessary Malloc for one-off API calls
const mallocedWords = CanvasKit.Malloc(Uint32Array, 2);
mallocedWords.toTypedArray().set([0, text.length]);
builder.setWordsUtf16(mallocedWords);

// Case 2: Missing CanvasKit.Free on allocated buffers
const mallocedBidis = CanvasKit.Malloc(Uint32Array, 3);
mallocedBidis.toTypedArray().set([0, text.length, 0]);
const paragraph = builder.buildWithClientInfo(mallocedBidis);
paragraph.delete();
// Leaked: CanvasKit.Free(mallocedBidis) omitted
```

Good:

```cpp
// Case 1: Prefer standard JS arrays (CanvasKit marshals and frees automatically)
const words = [0, text.length];
builder.setWordsUtf16(words);

// Case 2: Explicitly free CanvasKit.Malloc allocations with CanvasKit.Free
const mallocedBidis = CanvasKit.Malloc(Uint32Array, 3);
mallocedBidis.toTypedArray().set([0, text.length, 0]);
const paragraph = builder.buildWithClientInfo(mallocedBidis);
paragraph.delete();
CanvasKit.Free(mallocedBidis);
```

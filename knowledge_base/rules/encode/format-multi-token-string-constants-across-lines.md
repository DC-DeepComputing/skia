---
name: skia/encode/format-multi-token-string-constants-across-lines
description: When passing multi-item null-delimited or fixed-length token sequences to C APIs, extract named constants and split concatenated string literals across lines
rubric: readability
severity: nit
---

**Rule.** When defining multi-item null-delimited or fixed-length token
sequences passed to C APIs (such as image chunk lists or metadata tag names),
extract them into named constants and split the tokens into concatenated string
literals across separate lines so each token is visually distinct. In addition,
define an explicit named constant for the item count rather than passing a raw
numeric literal.

**Why.** Packing multiple fixed-width or null-delimited tokens into a single
unbroken string literal obscures individual items and makes verification against
format specifications difficult. Declaring named constants and splitting tokens
across lines makes individual elements immediately recognizable and eliminates
magic number counts at call sites.

## Examples

Bad:

```cpp
png_set_keep_unknown_chunks(fPngPtr, PNG_HANDLE_CHUNK_ALWAYS,
                            (png_const_bytep)"gmAP\0gdAT\0mDCV\0cLLI\0", 4);
```

Good:

```cpp
static constexpr char kHdrChunkNames[] = "gmAP\0"
                                         "gdAT\0"
                                         "mDCV\0"
                                         "cLLI\0";
static constexpr int kNumHdrChunkNames = 4;
png_set_keep_unknown_chunks(fPngPtr, PNG_HANDLE_CHUNK_ALWAYS,
                            (png_const_bytep)kHdrChunkNames, kNumHdrChunkNames);
```

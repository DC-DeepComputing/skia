---
name: skia/name-buffer-capacity-queries-with-unit-suffix
description: When declaring buffer variables, parameters, or capacity query methods, use explicit unit suffixes like Bytes, Count, or Words to distinguish byte sizes from element counts
rubric: readability
severity: should_fix
---

**Rule.** When declaring variables, parameters, or query methods for buffers,
spans, or allocators containing multi-byte elements, use explicit unit suffixes
or count nouns (such as Bytes, Count, IndexCount, or numWords) rather than
generic names like size, length, or remaining(). Distinguish raw byte sizes from
element, word, or index counts, and unaligned byte capacities from
alignment-adjusted slots.

**Why.** Ambiguous identifiers like dataLength, size, or remaining() for buffers
of multi-byte types leave it unclear whether the quantity represents raw bytes
or element counts. Explicit unit suffixes make the metric immediately evident to
callers, preventing sizing mismatches, indexing errors, and buffer overruns when
passing counts to byte-oriented APIs or GPU allocators.

## Examples

Bad:

```cpp
// Ambiguous: unclear whether remaining() returns bytes, elements, or slots.
class BufferSubAllocator {
public:
    uint32_t remaining() const {
        return fBuffer ? SkTo<uint32_t>(fBuffer->size()) - fOffset : 0;
    }
};

// Ambiguous: dataLength obscures whether the quantity represents uint32_t words or bytes.
const int dataLength = keyData[currentIndex++];
```

Good:

```cpp
// Explicit: unit suffix clarifies that the quantity is an unaligned byte count.
class BufferSubAllocator {
public:
    uint32_t remainingBytes() const {
        return fBuffer ? SkTo<uint32_t>(fBuffer->size()) - fOffset : 0;
    }
};

// Explicit: clarifies that the quantity is an element/word count for typed buffer indexing.
const int dataIndexCount = keyData[currentIndex++];
```

---
name: skia/prefer-int-for-sizes-and-offsets
description: When declaring container sizes, buffer offsets, or collection indices, prefer int over size_t—never assign negative sentinels to unsigned types—and keep external APIs unsigned
rubric: correctness
severity: should_fix
---

**Rule.** Prefer signed int over size_t or unsigned types for internal buffer
offsets, byte sizes, container counts, and collection indices (matching
skia_private::TArray), using -1 as the invalid sentinel and SkTo<int>() when
narrowing. Never assign negative values like -1 to unsigned types or size_t for
sentinels; use signed int with -1, or an explicit maximum constant
(SIZE_MAX/UINT32_MAX) if unsigned is required. Conversely, when consuming
external APIs or FFI structs returning unsigned types, keep local variables and
loop counters unsigned to avoid redundant casts and comparison warnings.

**Why.** Skia prefers signed int for internal sizes, indices, and offsets to
eliminate 64-bit storage bloat, match Skia container size() return types, and
allow using -1 as a sentinel. Assigning -1 to unsigned types causes implicit
modular wraparound, triggers -Wsign-conversion warnings, and creates comparison
hazards. Conversely, forcing signed conversions on external APIs or FFI types
that produce unsigned values introduces visual clutter, redundant static_cast
boilerplate, and signed/unsigned comparison warnings.

## Examples

Bad:

```cpp
// Bad: internal collection index uses size_t with negative or mismatched sentinel
using SpecializationIndex = size_t;
static constexpr SpecializationIndex kUnspecialized = -1; // or UINT32_MAX

// Bad: internal buffer/container size uses size_t, forcing unverified casts
void rewindTo(size_t size) {
    fStorage.resize(static_cast<int>(size));
    fStartingOffset = size; // fStartingOffset is int
}
size_t size() const { return fStorage.size(); } // fStorage is SkTDArray<char>

// Bad: unnecessary static_cast from external unsigned FFI API
const int32_t dstRowStart = static_cast<int32_t>(rowsInfo.dst_row_start);
for (int32_t i = 0; i < rowCount; ++i) { ... }
```

Good:

```cpp
// Good: use int for collection index alias with -1 as sentinel
using SpecializationIndex = int;
static constexpr SpecializationIndex kUnspecialized = -1;

// Good: match Skia container int sizing, or use SkTo<int>() when narrowing
int size() const { return fStorage.size(); }
void rewindTo(int size) {
    fStorage.resize(size);
    fStartingOffset = size;
}
fStartingOffset = SkTo<int>(externalSize);

// Good: preserve external unsigned API types without redundant casts
const uint32_t dstRowStart = rowsInfo.dst_row_start;
for (uint32_t i = 0; i < rowCount; ++i) { ... }
```

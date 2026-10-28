---
name: skia/codec/use-skassert-release-for-codec-invariants
description: When asserting buffer bounds, slice capacities, or decoder contract postconditions, use SkASSERT_RELEASE instead of debug-only SkASSERT or defensive input errors
rubric: readability
severity: should_fix
---

**Rule.** In image decoders and codecs processing untrusted input, use
SkASSERT_RELEASE instead of debug-only SkASSERT to assert buffer bounds, slice
capacities, and critical decoding invariants so checks fail safely in release
builds. Additionally, enforce internal decoder contract postconditions and FFI
invariants (such as metadata being loaded after a successful initialization
call) with SkASSERT_RELEASE rather than defensively returning runtime input
errors (such as kErrorInInput or kInvalidInput).

**Why.** Image codecs parse untrusted external data; because SkASSERT is
compiled out in release builds, relying on it for bounds checks leaves
production vulnerable to out-of-bounds memory access. Conversely, postconditions
guaranteed by successful upstream contracts represent internal implementation
invariants rather than malformed input; returning runtime input errors masks
crate or FFI glue bugs as data corruption instead of failing fast.

## Examples

Bad:

```cpp
// Debug-only assertion is compiled out in release builds, risking memory corruption.
SkASSERT(fullImageBuffer.size() >= dst.size());

// Defensive input error masks implementation or FFI glue bugs as malformed stream data.
if (!reader->metadata_loaded()) {
    *result = kErrorInInput;
    return nullptr;
}
```

Good:

```cpp
// SkASSERT_RELEASE aborts safely in production if untrusted input violates bounds.
SkASSERT_RELEASE(fullImageBuffer.size() >= dst.size());

// Contract postcondition fails fast on internal bugs instead of misreporting input error.
SkASSERT_RELEASE(reader->metadata_loaded());
```

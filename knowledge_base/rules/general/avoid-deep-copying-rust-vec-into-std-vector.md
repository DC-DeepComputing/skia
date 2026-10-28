---
name: skia/avoid-deep-copying-rust-vec-into-std-vector
description: When transferring buffers across Rust CXX FFI boundaries, move rust::Vec or pass a mutable rust::Slice rather than deep-copying into std::vector
rubric: safety
severity: nit
---

**Rule.** Do not deep-copy buffers across the CXX FFI boundary by assigning or
copying a rust::Vec<T> into a std::vector<T>. Instead, take ownership of the
Rust buffer directly by moving it (std::move) into a rust::Vec<T> member, or
pass a mutable rust::Slice<T> so Rust can write directly into caller-allocated
memory without intermediate allocations.

**Why.** Copying elements from rust::Vec<T> into std::vector<T> via .assign(),
range inserts, or copy construction allocates redundant heap memory and performs
unnecessary buffer copies. CXX's rust::Vec<T> supports move semantics in C++ for
zero-copy ownership transfer, while passing rust::Slice<T> allows Rust routines
to write directly into destination memory.

## Examples

Bad:

```cpp
// Redundantly copying a rust::Vec into a std::vector:
fDecodingState->fRustDecodedData.assign(
    decode_result.pixels.begin(),
    decode_result.pixels.end()
);
```

Good:

```cpp
// Either take ownership of the rust::Vec buffer directly via std::move:
fDecodingState->fRustDecodedData = std::move(decode_result.pixels);

// Or pass a destination rust::Slice for Rust to write directly into caller memory:
rust::Slice<uint8_t> rowSlice(rowSpan);
fReader->next_row(rowSlice);
```

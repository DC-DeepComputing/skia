---
name: skia/rust/codec-zero-initialize-incremental-decode-buffers
description: When allocating or resizing intermediate pixel buffers in incremental or streaming image decoders, zero-initialize them rather than using uninitialized capacity and unsafe set_len
rubric: safety
severity: should_fix
---

**Rule.** In incremental or streaming image decoders and FFI bridges, do not
bypass buffer zero-initialization by allocating uninitialized capacity and
calling `unsafe set_len()`. Explicitly zero-initialize intermediate decode
buffers (such as using `vec![0u8; len]` or `.resize(len, 0)`) or strictly slice
the returned buffer to fully decoded rows. Ensure unwritten bytes never expose
uninitialized memory on `IncompleteInput` or violate `fZeroInitialized` contract
expectations.

**Why.** In streaming or progressive decoding, input data can be truncated,
causing decode routines to exit early with `IncompleteInput`. Preallocating
uninitialized capacity and advancing buffer length via `set_len()` leaves
trailing unwritten bytes uninitialized; in Rust, creating slices over
uninitialized memory is immediate undefined behavior, and passing indeterminate
memory across FFI violates SkCodec's `fZeroInitialized` guarantees.

## Examples

Bad:

```cpp
// BAD: Preallocates uninitialized capacity and advances length before decoding.
// On partial decode (IncompleteInput), unwritten trailing bytes remain uninitialized.
if self.image_data.capacity() < total_bytes {
    self.image_data = Vec::with_capacity(total_bytes);
}
unsafe {
    self.image_data.set_len(total_bytes);
}
match decoder.read_image_data(&mut self.image_data) { ... }
```

Good:

```cpp
// GOOD: Zero-initialize buffer so unwritten bytes on IncompleteInput are safe zeros.
if self.image_data.len() < total_bytes {
    self.image_data.clear();
    self.image_data.resize(total_bytes, 0);
}
match decoder.read_image_data(&mut self.image_data) { ... }
```

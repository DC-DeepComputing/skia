---
name: skia/rust/return-borrowed-slices-via-cxx-out-param
description: In Rust CXX bridges returning borrowed slices alongside metadata, use a slice out-parameter (&mut &'a [u8]) instead of multiple temporally coupled sequential methods
rubric: design
severity: nit
---

**Rule.** In Rust CXX FFI bridges (#[cxx::bridge]), avoid splitting an operation
into coordinated sequential methods that require intermediate state tracking
(such as one method to advance or decode and a second method to fetch the
resulting data). When CXX limitations prevent returning composite structs or
tuples containing borrowed slices with lifetimes, pass an out-parameter
reference to a slice (buffer: &mut &'a [u8]) alongside the metadata return type
in a single call.

**Why.** Splitting an operation into temporally coupled FFI calls forces the
Rust reader or decoder to retain transient intermediate state, complicates the
C++ caller lifecycle, and requires runtime assertions or panics to guard against
out-of-order calls. Passing a borrowed slice via an out-parameter (&mut &'a
[u8], bridging to rust::Slice<const uint8_t>& in C++) allows combining metadata
retrieval and zero-copy slice borrowing into a single stateless, atomic
invocation.

## Examples

Bad:

```cpp
#[cxx::bridge]
mod ffi {
    extern "Rust" {
        type Reader;
        // Anti-pattern: Two temporally coupled methods requiring intermediate state:
        fn consume_next_rows(self: &mut Reader) -> DecodedRowsInfo;
        fn get_image_data(self: &mut Reader) -> &[u8];
    }
}
```

Good:

```cpp
#[cxx::bridge]
mod ffi {
    extern "Rust" {
        type Reader;
        // Single call returning metadata and borrowing slice via out-parameter:
        unsafe fn get_next_rows<'a>(
            self: &'a mut Reader,
            buffer: &mut &'a [u8],
        ) -> DecodedRowsInfo;
    }
}
```

---
name: skia/rust/accept-skstreamadapter-in-codec-ffi
description: In Rust SkCodec implementations and FFI bridges, stream directly via SkStreamAdapter instead of copying streams to SkData or raw byte slices
rubric: correctness
severity: should_fix
---

**Rule.** In Rust SkCodec implementations and CXX FFI bridges, stream directly
from SkStream using rust::stream::SkStreamAdapter (from
rust/common/SkStreamAdapter.h) rather than eagerly copying or buffering the
stream to SkData or raw byte slices (&[u8]). When defining FFI boundaries,
accept Pin<&mut SkStreamAdapter> for transient inspection (such as format
sniffing and directory parsing) to enable C++ stack allocation, and accept
UniquePtr<SkStreamAdapter> for operations that transfer stream ownership to a
persistent Rust reader.

**Why.** Skia codecs operate over dynamic SkStream sources that stream
incrementally or seek without buffering the full stream into memory. Eagerly
copying or reading the entire stream into an in-memory buffer upfront breaks
progressive decoding on incomplete inputs (such as network streams) and incurs
unnecessary allocation and copying overhead. Passing SkStreamAdapter connects
SkStream directly to Rust's Read and Seek traits, while taking Pin<&mut
SkStreamAdapter> allows C++ callers to stack-allocate temporary streams and
adapters for transient calls.

## Examples

Bad:

```cpp
// Eagerly copying stream to SkData or taking raw byte slices in FFI
sk_sp<SkData> data = SkCopyStreamToData(stream.get());
rust::Slice<const uint8_t> slice{
    reinterpret_cast<const uint8_t*>(data->data()), data->size()};

// FFI: Taking raw byte slices or forcing heap-allocated UniquePtr for transient inspection
#[cxx::bridge]
mod ffi {
    extern "Rust" {
        fn is_ico(data: &[u8]) -> bool;
        fn parse_directory(input: UniquePtr<SkStreamAdapter>) -> Box<DirectoryResult>;
    }
}
```

Good:

```cpp
// Streaming directly via SkStreamAdapter and passing UniquePtr for reader ownership
auto adapter = std::make_unique<rust::stream::SkStreamAdapter>(stream.get());
rust::Box<rust_bmp::ResultOfReader> result = rust_bmp::new_reader(std::move(adapter));

// FFI: Pin<&mut T> for non-consuming inspection; UniquePtr<T> for reader ownership
#[cxx::bridge]
mod ffi {
    extern "Rust" {
        fn is_ico(input: Pin<&mut SkStreamAdapter>) -> bool;
        fn new_reader(input: UniquePtr<SkStreamAdapter>) -> Box<Reader>;
    }
}
// Caller stack-allocates transient streams and adapters
SkMemoryStream stream(std::move(data));
rust::stream::SkStreamAdapter stackAdapter(&stream);
return rust_ico::is_ico(stackAdapter);
```

---
name: skia/codec/delegate-decoder-format-detection-to-codec
description: When implementing image decoder wrappers (Sk<Format>Decoder), delegate format detection (Is<Format>) to the underlying codec implementation instead of calling backend or FFI routines directly
rubric: design
severity: should_fix
---

**Rule.** In image decoder wrappers (`Sk<Format>Decoder` or
`Sk<Format>RustDecoder`), delegate format sniffing (`Is<Format>`) to the
underlying codec implementation (such as `Sk<Format>Codec::Is<Format>`) rather
than invoking backend parsing libraries or FFI functions directly. Keep decoder
registration wrappers decoupled from third-party bridge headers and direct FFI
dependencies.

**Why.** Decoder wrappers act as lightweight registration entry points for
`SkCodecs::Register`. Calling backend or FFI functions directly in the wrapper
requires extra header includes and forces the decoder's build target (such as
`BUILD.bazel`) to depend directly on internal FFI bridges and third-party
runtime crates, violating target layering.

## Examples

Bad:

```cpp
#include "experimental/rust_ico/ffi/FFI.rs.h"
#include "rust/common/SkStreamAdapter.h"

bool IsIco(const void* buff, size_t bytesRead) {
    auto data = SkData::MakeWithoutCopy(buff, bytesRead);
    auto stream = std::make_unique<SkMemoryStream>(std::move(data));
    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(stream.get());
    return rust_ico::is_ico(std::move(inputAdapter));
}
```

Good:

```cpp
#include "experimental/rust_ico/decoder/impl/SkIcoRustCodec.h"

bool IsIco(const void* buff, size_t bytesRead) {
    return SkIcoRustCodec::IsIco(buff, bytesRead);
}
```

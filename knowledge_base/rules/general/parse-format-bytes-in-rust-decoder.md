---
name: skia/parse-format-bytes-in-rust-decoder
description: When wrapping Rust decoders or parsers in C++, parse format header bytes inside Rust and expose structured properties rather than indexing raw bytes in C++
rubric: design
severity: should_fix
---

**Rule.** Do not inspect, unpack, or manipulate raw format header bytes in C++
wrapper code when integrating a Rust parser or decoder. Perform all format
parsing, header inspection, and byte validation inside the Rust implementation,
exposing structured properties, enums, or query methods across the FFI boundary
instead.

**Why.** Format-specific header parsing and raw byte manipulation belong inside
the decoder/parser layer, not in the C++ codec wrapper. Delegating all parsing
to the Rust implementation maintains clean architectural layering, prevents
brittle ad-hoc byte indexing in C++, and upholds the memory-safety guarantee of
parsing untrusted input formats in memory-safe code.

## Examples

Bad:

```cpp
// Bad: Inspecting raw BMP header bytes in C++ wrapper to determine row order
if (data->size() >= 26) {
    const uint8_t* rawData = reinterpret_cast<const uint8_t*>(data->data());
    int32_t heightSigned = static_cast<int32_t>(
        rawData[22] | (rawData[23] << 8) | (rawData[24] << 16) | (rawData[25] << 24)
    );
    if (heightSigned < 0) {
        rowOrder = SkCodec::kTopDown_SkScanlineOrder;
    }
}
```

Good:

```cpp
// Good: Rust decoder parses header and exposes structured property across FFI
// In Rust decoder / FFI:
pub fn bottom_up(&self) -> bool {
    self.bottom_up
}

// In C++ SkCodec wrapper:
SkCodec::SkScanlineOrder rowOrder = reader->bottom_up()
    ? SkCodec::kBottomUp_SkScanlineOrder
    : SkCodec::kTopDown_SkScanlineOrder;
```

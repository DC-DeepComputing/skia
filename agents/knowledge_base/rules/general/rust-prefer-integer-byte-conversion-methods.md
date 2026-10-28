---
name: skia/rust-prefer-integer-byte-conversion-methods
description: When serializing or deserializing integers to or from bytes in Rust, use standard to_le_bytes or from_le_bytes methods instead of bitwise shifts and casting
rubric: readability
severity: should_fix
---

**Rule.** When converting integers to or from raw bytes in Rust (such as during
binary serialization or deserialization), use the standard library integer
byte-conversion methods (`to_le_bytes()`, `to_be_bytes()`, `from_le_bytes()`, or
`from_be_bytes()`). Do not open-code manual bitwise shifts (such as `>> 8` or
`<< 8`), masking (`& 0xFF`), and truncating casts (`as u8`).

**Why.** Standard integer conversion methods explicitly document endianness
intent and map directly to optimized byte-reversal or no-op instructions. Manual
bitwise shifting and casting are verbose, error-prone, and risk subtle
truncation or sign-extension bugs.

## Examples

Bad:

```cpp
// Serializing integer to bytes:
let mut bytes = Vec::new();
for value in values {
    bytes.push(value as u8);
    bytes.push((value >> 8) as u8);
}

// Deserializing bytes to integer:
let val = (bytes[0] as u16) | ((bytes[1] as u16) << 8);
```

Good:

```cpp
// Serializing integer to bytes:
let mut bytes = Vec::new();
for value in values {
    bytes.extend(value.to_le_bytes());
}

// Deserializing bytes to integer:
let val = u16::from_le_bytes(bytes[0..2].try_into().unwrap());
```

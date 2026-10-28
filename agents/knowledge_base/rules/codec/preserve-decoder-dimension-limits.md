---
name: skia/codec/preserve-decoder-dimension-limits
description: When configuring image decoder options or bridges, do not override dimension limits to unbounded values like usize::MAX; preserve library defaults or format bounds
rubric: safety
severity: should_fix
---

**Rule.** When configuring third-party image decoders or codec FFI bridges, do
not override decoder dimension limits to unbounded values (such as usize::MAX or
SIZE_MAX). Retain the decoding library's default limits or enforce
format-specific bounds (such as JPEG's 64K limit) to prevent resource exhaustion
from untrusted inputs.

**Why.** Image decoders process untrusted input streams. Overriding dimension
limits to unbounded maximums disables the underlying library's protections
against decompression bombs and excessive memory allocation, exposing the host
process to denial-of-service vulnerabilities.

## Examples

Bad:

```cpp
let options = zune_core::options::DecoderOptions::default()
    .set_strict_mode(false)
    .set_max_width(usize::MAX)
    .set_max_height(usize::MAX);
```

Good:

```cpp
let options = zune_core::options::DecoderOptions::default()
    .set_strict_mode(false);
```

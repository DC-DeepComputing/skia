---
name: skia/prefer-sk-bit-cast-over-std-bit-cast
description: When type-punning or bit-casting values, use sk_bit_cast instead of std::bit_cast to preserve compatibility with supported GCC 10 toolchains
rubric: correctness
severity: blocking
---

**Rule.** Do not use std::bit_cast; use Skia's sk_bit_cast helper (from
src/core/SkUtils.h) instead. When converting representations between same-sized
trivially copyable types, use sk_bit_cast to maintain build compatibility across
all supported compiler baselines.

**Why.** std::bit_cast was introduced in C++20 but was not implemented in
libstdc++ until GCC 11. Because Skia supports toolchains based on GCC 10, direct
use of std::bit_cast causes build failures; sk_bit_cast provides a portable
equivalent across all supported compilers.

## Examples

Bad:

```cpp
#include <bit>

return std::bit_cast<uint64_t>(*this) == std::bit_cast<uint64_t>(other);
```

Good:

```cpp
#include "src/core/SkUtils.h"

return sk_bit_cast<uint64_t>(*this) == sk_bit_cast<uint64_t>(other);
```

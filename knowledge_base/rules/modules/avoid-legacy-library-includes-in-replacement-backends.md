---
name: skia/modules/avoid-legacy-library-includes-in-replacement-backends
description: When implementing alternative or replacement modular backends, do not include headers from legacy or mutually exclusive libraries; define local helpers or constants instead
rubric: design
severity: should_fix
---

**Rule.** In independent, modular, or replacement backend implementations (such
as Rust-based image codecs, Fontations, or ICU4X in skunicode), do not #include
headers from alternative, legacy, or mutually exclusive libraries (such as
<png.h>, <ft2build.h>, or classic ICU's <unicode/umachine.h>). Keep modular
backend source files strictly isolated from unrelated backend dependencies, and
define standalone local constants, helpers, or types instead of importing them
from foreign backends.

**Why.** Replacement and modular backends are designed to provide memory-safe,
isolated, or swappable implementations. Including headers from alternative or
legacy libraries pulls in unwanted transitive build dependencies, couples
mutually exclusive backends, and risks One Definition Rule (ODR) or macro/symbol
conflicts.

## Examples

Bad:

```cpp
// In experimental/rust_png/decoder/impl/SkPngRustCodec.cpp (Rust PNG codec):
#include <png.h>
if (std::abs(gamma - 1.0f) <= PNG_GAMMA_THRESHOLD_FIXED) { ... }

// In modules/skunicode/src/SkUnicode_icu4x.cpp (ICU4X backend):
#include <unicode/umachine.h>
#include "modules/skunicode/include/SkUnicode_icu4x.h"
```

Good:

```cpp
// In experimental/rust_png/decoder/impl/SkPngRustCodec.cpp:
// Define local constant to avoid coupling to libpng headers
constexpr float kPngGammaThreshold = 0.05f;
if (std::abs(gamma - 1.0f) <= kPngGammaThreshold) { ... }

// In modules/skunicode/src/SkUnicode_icu4x.cpp:
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#include <ICU4XBidi.hpp>
```

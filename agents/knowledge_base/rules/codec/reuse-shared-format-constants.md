---
name: skia/codec/reuse-shared-format-constants
description: When decoding or encoding format-specific metadata across codec backends, reuse shared constants from private headers instead of hardcoding magic numbers
rubric: readability
severity: nit
---

**Rule.** When implementing image decoding or encoding logic across codec
backends (such as C++ and Rust decoders or encoders), reuse established format
constants from shared private headers (such as kGraySigBit_GrayAlphaIsJustAlpha
in src/codec/SkPngPriv.h) instead of hardcoding raw numeric literals. Mirroring
format conventions using shared constants documents the protocol invariant and
prevents behavioral divergence between backends.

**Why.** Image codecs often define internal conventions or sentinel values (such
as signaling an alpha-only grayscale PNG via a specific sBIT value) that must
match across encoders and decoders. Hardcoding magic numbers obscures why the
value was chosen, risks divergence across alternative backends, and makes
format-specific protocols difficult to track and maintain.

## Examples

Bad:

```cpp
// In alternative PNG decoder backend:
if (sBit.size() == 2 && reader.output_bits_per_component() == 8) {
    // Hardcoding magic number 1 obscures Skia's internal alpha-only convention
    if (sBit[0] == 1 && sBit[1] == 8) {
        return SkEncodedInfo::kXAlpha_Color;
    }
}
```

Good:

```cpp
#include "src/codec/SkPngPriv.h"

// In alternative PNG decoder backend:
if (sBit.size() == 2 && reader.output_bits_per_component() == 8) {
    if (sBit[0] == kGraySigBit_GrayAlphaIsJustAlpha && sBit[1] == 8) {
        return SkEncodedInfo::kXAlpha_Color;
    }
}
```

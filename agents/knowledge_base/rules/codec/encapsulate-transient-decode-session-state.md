---
name: skia/codec/encapsulate-transient-decode-session-state
description: When structuring codec decode state, encapsulate transient session parameters in a per-operation struct, factoring buffer descriptors for shared helpers while avoiding redundant single-member nested wrappers
rubric: design
severity: should_fix
---

**Rule.** Store transient decode-session parameters (such as subset bounds, row
indices, and buffer offsets) in a dedicated per-operation session state struct
(e.g., DecodingState) rather than codec member variables to keep the codec
stateless. Factor destination buffer layout into a dedicated descriptor struct
(e.g., DecodingDstInfo) when row-processing helpers consume buffer geometry
independently or when session state tracks additional progress fields. When the
session state contains no other fields and helpers do not require a separate
descriptor, flatten the buffer span and stride directly into the session state
struct instead of nesting a redundant single-member wrapper.

**Why.** Storing transient decode-session parameters on codec member variables
risks stale data leaks or corrupted state across multiple frame or subset
decodes. Factoring buffer layout into a descriptor allows row-processing helpers
to operate on arbitrary intermediate buffers without fabricating monolithic
state instances. Conversely, nesting a dedicated descriptor inside an otherwise
empty session struct adds unnecessary syntactic indirection and gratuitous
abstraction when simple direct fields suffice.

## Examples

Bad:

```cpp
// 1. Storing transient decode session state as codec member variables:
class SkPngRustCodec : public SkCodec {
    int fFirstRow = 0;
    size_t fYByteOffset = 0;
};

// 2. Redundant single-member nested struct wrapper with no other session fields:
struct DecodingState {
    struct DecodingDstInfo {
        SkSpan<uint8_t> fDst;
        size_t fDstRowStride = 0;
    } fDecodingDstInfo;
};
```

Good:

```cpp
// 1. Transient session state encapsulates progress fields; helpers take descriptor:
struct DecodingState {
    DecodingDstInfo fDstInfo;
    int fFirstRow = 0;
    size_t fYByteOffset = 0;
};
void expandRow(..., const DecodingDstInfo& dstInfo);

// 2. When session state has no other fields, flatten buffer layout directly:
struct DecodingState {
    SkSpan<uint8_t> fDst;
    size_t fDstRowStride = 0;
};
```

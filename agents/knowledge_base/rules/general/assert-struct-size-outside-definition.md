---
name: skia/assert-struct-size-outside-definition
description: When designing packed structs or refactoring data layouts, document size rationales and enforce constraints with static_assert(sizeof(T) == Expected) outside the definition
rubric: readability
severity: should_fix
---

**Rule.** When designing structs or classes with intentional size or packing
constraints (such as fitting within a 32-bit word, matching a storage format, or
optimizing cache locality and sorting), document the rationale for the
constraint and enforce it using static_assert(sizeof(T) == ExpectedSize)
immediately outside and following the struct definition rather than relying
solely on comments. Never verify struct size by summing member sizes inside the
struct body, and do not remove or bypass existing static_assert(sizeof(T) ==
...) checks during refactorings unless the size is intentionally changing.

**Why.** Comments alone do not prevent accidental layout bloat, and summing
member sizes inside the struct body ignores compiler alignment and padding
bytes. Documenting the technical rationale clarifies why strict layout bounds
exist (such as cache locality or sorting throughput), while placing
static_assert(sizeof(T) == ExpectedSize) outside the definition catches
unexpected padding, field reordering, or new members at compile time.

## Examples

Bad:

```cpp
// Bad: Relying on comments without static_assert, or lacking rationale
struct AttachmentDesc {
    TextureFormat fFormat = TextureFormat::kUnsupported;
    LoadOp fLoadOp = LoadOp::kDiscard;
    StoreOp fStoreOp = StoreOp::kDiscard;
    // GPU-supported sample counts fit in a byte; AttachmentDesc stays at 32-bits.
    uint8_t fSampleCount = 1;
};

// Bad: Undocumented size requirement and summing member sizes inside struct
struct Tile {
    uint16_t x;
    uint16_t y;
    uint32_t fPackedLineIdx;
    static_assert(sizeof(x) + sizeof(y) + sizeof(fPackedLineIdx) == 8);
};
```

Good:

```cpp
// Good: Document packing rationale and enforce with static_assert outside struct
// Sized to 32 bits to match GPU descriptor register layout and packing requirements.
struct AttachmentDesc {
    TextureFormat fFormat = TextureFormat::kUnsupported;
    LoadOp fLoadOp = LoadOp::kDiscard;
    StoreOp fStoreOp = StoreOp::kDiscard;
    uint8_t fSampleCount = 1;
};
static_assert(sizeof(AttachmentDesc) == sizeof(uint32_t));

// Good: Document sorting/alignment rationale and verify full struct size outside
// Aligned to 8 bytes for efficient sorting and cache locality across tiles.
struct Tile {
    uint16_t x;
    uint16_t y;
    uint32_t fPackedLineIdx;
};
static_assert(sizeof(Tile) == 8);
```

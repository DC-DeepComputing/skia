---
name: skia/codec/consolidate-variant-stream-factories
description: When implementing codec factory methods across format variants or input sources, consolidate public factories using discriminating enums and factor shared reader metadata extraction into helpers
rubric: design
severity: should_fix
---

**Rule.** When image codec factory methods share stream setup across format
variants (such as standalone vs. container-embedded streams), consolidate them
into a single factory taking a discriminating enum parameter with a default
value. Across multiple factory entry points (such as stream, data, or variant
paths), factor common reader metadata extraction, SkEncodedInfo construction,
and dimension or row bytes overflow validation into a shared helper (such as
MakeFromReader) rather than duplicating reader setup and validation across each
factory.

**Why.** Duplicating factory methods and reader setup across format variants or
input sources introduces code drift, bloats internal APIs, and risks
inconsistent metadata extraction or overflow validation. Parameterizing format
variants with an enum and centralizing reader initialization in a shared helper
keeps public interfaces clean and ensures uniform validation across all creation
paths.

## Examples

Bad:

```cpp
// Separate public factories duplicating stream setup and reader metadata extraction
std::unique_ptr<SkBmpRustCodec> SkBmpRustCodec::MakeFromStream(
        std::unique_ptr<SkStream> stream, Result* result) {
    auto reader = ...;
    SkEncodedInfo info = SkEncodedInfo::Make(reader->width(), reader->height(), ...);
    // ... validate row bytes and construct codec ...
}

std::unique_ptr<SkBmpRustCodec> SkBmpRustCodec::MakeFromIcoStream(
        std::unique_ptr<SkStream> stream, Result* result) {
    auto reader = ...;
    SkEncodedInfo info = SkEncodedInfo::Make(reader->width(), reader->height(), ...);
    // ... duplicate row bytes validation and construct codec ...
}
```

Good:

```cpp
enum class StreamType { kBMP, kICO };

// Public factory consolidates variants via discriminating enum with default argument
std::unique_ptr<SkBmpRustCodec> SkBmpRustCodec::MakeFromStream(
        std::unique_ptr<SkStream> stream, Result* result, StreamType type = StreamType::kBMP) {
    auto reader = ...;
    return MakeFromReader(std::move(stream), std::move(reader), type, result);
}

// Shared internal helper encapsulates reader metadata extraction and validation
std::unique_ptr<SkBmpRustCodec> SkBmpRustCodec::MakeFromReader(
        std::unique_ptr<SkStream> stream, rust::Box<rust_bmp::Reader> reader,
        StreamType type, Result* result) {
    SkEncodedInfo info = SkEncodedInfo::Make(reader->width(), reader->height(), ...);
    // ... validate row bytes overflow and construct codec ...
}
```

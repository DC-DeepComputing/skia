---
name: skia/avoid-copying-memory-backed-streams
description: When extracting SkData from an SkStream, reuse stream->getData() or shareSubset(); only use SkData::MakeWithoutCopy if the stream outlives the returned SkData
rubric: safety
severity: blocking
---

**Rule.** When extracting SkData or slicing embedded payloads from an SkStream,
reuse stream->getData() or data->shareSubset() instead of deep-copying when
ref-counted data is available. When falling back to stream->getMemoryBase(),
only use SkData::MakeWithoutCopy() if the underlying stream is guaranteed to
outlive the returned SkData (such as during transient inspection); never use
MakeWithoutCopy() when the SkData is retained in a long-lived object (such as
SkTypeface)—use SkData::MakeWithCopy() or SkData::MakeFromStream() instead.

**Why.** stream->getData() and data->shareSubset() reuse existing ref-counted
SkData without redundant heap allocations or copies. SkData::MakeWithoutCopy()
does not extend the lifetime of the underlying stream; wrapping a temporary
stream's memory base in a long-lived SkData leaves dangling pointers and causes
use-after-free crashes.

## Examples

Bad:

```cpp
// Slicing container stream with deep copies instead of shareSubset
dataStream->seek(offset);
auto entry = std::make_unique<SkMemoryStream>(SkData::MakeFromStream(dataStream.get(), size));

// Using MakeWithoutCopy when SkData outlives the stream (e.g. stored in SkTypeface)
if (font_data->getMemoryBase() && font_data->getLength()) {
    return SkData::MakeWithoutCopy(font_data->getMemoryBase(), font_data->getLength());
}
```

Good:

```cpp
// Reuse stream->getData() first, and carve zero-copy slices with shareSubset
sk_sp<const SkData> entryData = data->shareSubset(offset, size);
auto entry = std::make_unique<SkMemoryStream>(entryData);

// Fall back to MakeWithCopy when the returned SkData outlives the stream
if (font_data->getData()) {
    return font_data->getData();
}
if (font_data->getMemoryBase() && font_data->getLength()) {
    return SkData::MakeWithCopy(font_data->getMemoryBase(), font_data->getLength());
}
return SkData::MakeFromStream(font_data.get(), font_data->getLength());
```
